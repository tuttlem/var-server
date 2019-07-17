
#include "./daemon.h"

pid_t vs_daemon_pid;
int vs_daemon_running;
int vs_listener;

int vs_port = 25052;
int vs_backlog = 32;

/* polling timeout is 3 minutes */
int vs_poll_timeout = (3 * 60 * 1000);

struct pollfd client_fds[200];
int n_client_fds = 1;

/**
 * Initialize the network server
 */
int server_init(int port, int backlog) {
  int rc = 0, on = 1;
  struct sockaddr_in6 addr;

  /* create an AF_INET6 stream socket as the main listener */
  if ((vs_listener = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    log_error("Unable to create main socket (errno=%d)", errno);
    return ERR_SRINIT;
  }

  /* allow the socket descriptor to be reusable */
  if ((rc = setsockopt(vs_listener, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) < 0) {
    log_error("Unable to setsockopt (errno=%d)", errno);
    return ERR_SRINIT;
  }

  /* set the socket to be non-blocking */
  if ((rc = ioctl(vs_listener, FIONBIO, (char *)&on)) < 0) {
    log_error("Unable to set non-blocking (errno=%d)", errno);
    return ERR_SRINIT;
  }

  /* bind an address to the socket */
  memset(&addr, 0, sizeof(addr));
  memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);

  if ((rc = bind(vs_listener, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
    log_error("Unable to bind socket (errno=%d)", errno);
    return ERR_SRINIT;
  }
  
  /* start listening */
  if ((rc = listen(vs_listener, backlog)) < 0) {
    log_error("Unable to listen (errno=%d)", errno);
    return ERR_SRINIT;
  }

  /* initialize the client file descriptor structure */
  memset(client_fds, 0, sizeof(client_fds));
  client_fds[0].fd = vs_listener;
  client_fds[0].events = POLLIN;

  return ERR_SUCCESS;
}

/**
 * Tears down the server
 */
int server_teardown() {
  int i;

  /* close down any listening socket */
  for (i = 0; i < n_client_fds; i ++) {
    if (client_fds[i].fd >= 0) {
      close(client_fds[i].fd);
    }
  }

  /* close the listener */
  close(vs_listener);
  vs_listener = 0;

  return ERR_SUCCESS;
}

/*
 * Daemon
 */

/**
 * The var server's signal handler 
 */
void daemon_signal_handler(int sig) {
  log_info("Server received signal %d", sig);

  switch (sig) {
    case SIGHUP:
    case SIGTERM:
      vs_daemon_running = 0;
      break;
  }
}


/**
 * Daemonizes this application so that it will run in the background
 */
int daemon_init() {

  vs_daemon_running = 0;

  /* first fork */
  pid_t pid = fork();

  if (pid == -1) {
    log_error("Unable to fork (errno=%d)", errno);
    return ERR_DMINIT;
  } else if (pid != 0) {
    _exit(0);
  }

  /* start a new session */
  if (setsid() == -1) {
    log_error("Unable to become session leader (errno=%d)", errno);
    return ERR_DMINIT;
  }

  /* fork again */
  signal(SIGHUP, SIG_IGN);
  pid = fork();

  if (pid == -1) {
    log_error("Unable to secondary fork (errno=%d)", errno);
    return ERR_DMINIT;
  } else if (pid == 0) {
    _exit(0);
  }

  vs_daemon_pid = pid;

  /* change to a known location */
  if (chdir("/") == -1) {
    log_error("Unable to change working directory (errno=%d)", errno);
    return ERR_DMINIT;
  }

  /* attach the signal handlers */
  signal(SIGCHLD, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGHUP, daemon_signal_handler);
  signal(SIGTERM, daemon_signal_handler);
  
  /* set the umask */
  umask(0);

  /* handle the standard file descriptors */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  if (open("/dev/null", O_RDONLY) == -1) {
    log_error("Unable to re-open stdin (errno=%d)", errno);
    return ERR_DMINIT;
  }

  if (open("/dev/null", O_WRONLY) == -1) {
    log_error("Unable to re-open stdout (errno=%d)", errno);
    return ERR_DMINIT;
  }

  if (open("/dev/null", O_RDWR) == -1) {
    log_error("Unable to re-open stderr (errno=%d)", errno);
    return ERR_DMINIT;
  }

  /* setup the server now */
  if (server_init(vs_port, vs_backlog) != ERR_SUCCESS) {
    log_error("Failed to setup the server; terminating daemon");
    daemon_teardown();
    return ERR_DMINIT;
  }

  vs_daemon_running = 1;

  return ERR_SUCCESS;
}

/**
 * Tears down the daemon process
 */
int daemon_teardown() {
  server_teardown();

  log_info("Killing server pid %d", vs_daemon_pid);
  kill(vs_daemon_pid, SIGTERM);

  return ERR_SUCCESS;
}

/**
 * Runs the daemon process
 */
int daemon_run() {
  int i, j, rc, current_size, client_sd;
  int close_conn, len, compress_required = 0;

  unsigned char buffer[1024];

  log_info("Daemon is running");

  /* keep going until the daemon is signalled */
  while (vs_daemon_running) {

    log_debug("Polling");

    /* poll available sockets, or timeout */
    if ((rc = poll(client_fds, n_client_fds, vs_poll_timeout)) < 0) {
      log_error("Failed to poll sockets (errno=%d)", errno);
      break;
    }

    /* the poll expired here, so just move to the next iteration */
    if (rc == 0) {
      continue;
    }

    current_size = n_client_fds;
    for (i = 0; i < current_size; i ++) {

      /* process any descriptor that returns POLLIN */
      if (client_fds[i].revents == 0) {
        continue;
      }

      /* not getting POLLIN is unexpected; so log and get out */
      if (client_fds[i].revents != POLLIN) {
        log_error("Unexpected value (revents=%d", client_fds[i].revents);
        vs_daemon_running = 0;
        break;
      }

      /* check if the listening socket is readable */
      if (client_fds[i].fd == vs_listener) {

        /* accept all of the incoming connections now */
        do {

          client_sd = accept(vs_listener, NULL, NULL);

          if (client_sd < 0) {

            if (errno != EWOULDBLOCK) {
              log_error("Failed to accept socket (errno=%d)", errno);
              vs_daemon_running = 0;
            }

            break;
          }

          client_fds[n_client_fds].fd = client_sd;
          client_fds[n_client_fds].events = POLLIN;
          n_client_fds ++;

        } while (client_sd != -1);

      } else {
        close_conn = 1;

        /* receive all of the incoming data on client connections */
        if ((rc = recv(client_fds[i].fd, buffer, sizeof(buffer), 0)) < 0) {
          
          if (errno != EWOULDBLOCK) {
            log_error("Failed to receive from socket (errno=%d)", errno);
            close_conn = 1;
          }

          break;
        }

        /* check if the connection has already been closed */
        if (rc == 0) {
          close_conn = 1;
          break;
        }

        /* data was received */
        len = rc;

        if ((rc = send(client_fds[i].fd, buffer, len, 0)) < 0) {
          close_conn = 1;
          break;
        }

        if (close_conn) {
          close(client_fds[i].fd);
          client_fds[i].fd = -1;
          compress_required = 1;
        }

      }

    }

    if (compress_required) {
      compress_required = 0;

      for (i = 0; i < n_client_fds; i ++) {
        if (client_fds[i].fd == -1) {
          for (j = i; j < n_client_fds; j ++) {
            client_fds[j].fd = client_fds[j + 1].fd;
          }

          i --;
          n_client_fds --;
        }
      }
    }

  }

  log_info("Daemon is stopping");

  return ERR_SUCCESS;
}
