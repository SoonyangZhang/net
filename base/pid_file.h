#pragma once
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
int write_pidfile(const char *pid_file);
int read_pidfile(const char *pid_file);
int remove_pidfile(const char *pid_file);
bool check_pid_running(const char *pid_file);

