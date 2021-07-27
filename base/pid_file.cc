#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include "logging/logging.h"
#include "pid_file.h"
int write_pidfile(const char *pid_file)
{
  FILE *f;
  int fd;
  int pid;

  if ( ((fd = open(pid_file, O_RDWR|O_CREAT, 0644)) == -1)
       || ((f = fdopen(fd, "r+")) == NULL) ) {
      fprintf(stderr, "Can't open or create %s.\n", pid_file ? pid_file : "(null)");
      return 0;
  }
  
#ifdef HAVE_FLOCK
  if (flock(fd, LOCK_EX|LOCK_NB) == -1) {
      fscanf(f, "%d", &pid);
      fclose(f);
      printf("Can't lock, lock is held by pid %d.\n", pid);
      return 0;
  }
#endif

  pid = getpid();
  if (!fprintf(f,"%d\n", pid)) {
      printf("Can't write pid , %s.\n", strerror(errno));
      close(fd);
      return 0;
  }
  fflush(f);

#ifdef HAVE_FLOCK
  if (flock(fd, LOCK_UN) == -1) {
      printf("Can't unlock pidfile %s, %s.\n", pidfile, strerror(errno));
      close(fd);
      return 0;
  }
#endif
  close(fd);

  return pid;
}
int read_pidfile(const char *pid_file)
{
  FILE *f;
  int pid;

  if (!(f=fopen(pid_file,"r")))
    return 0;
  fscanf(f,"%d", &pid);
  fclose(f);
  return pid;
}
int remove_pidfile(const char *pid_file)
{
  return unlink (pid_file);
}
/* Return the running state */
bool is_pid_running(const char *pid_file)
{
    FILE *file = fopen(pid_file, "r");
    pid_t pid;

    /* pid_file not exist */
    if (!file)
        return false;

    if (fscanf(file, "%d", &pid) != 1) {
        fclose(file);
        return false;
    }
    fclose(file);

    /* remove pidfile if no process attached to it */
    if (kill(pid, 0)) {
        DLOG(INFO)<<"Remove a zombie pid file:"<<pid_file;
        remove_pidfile(pid_file);
        return false;
    }

    return true;
}
