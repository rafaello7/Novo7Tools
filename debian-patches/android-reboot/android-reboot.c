/*
 * Based on "halt" program by Miquel van Smoorenburg
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <utmp.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <initreq.h>
#include <linux/reboot.h>

static char progname[] = "android-reboot";
static int got_alrm = 0;

extern int ifdown(void);
extern int hdflush(void);

/*
 *	Send usage message.
 */
void usage(void)
{
	fprintf(stderr, "usage: %s [-n] [-f] [-i] <restart mode>\n", progname);
	fprintf(stderr, "\t-n: don't sync before restarting the system\n");
	fprintf(stderr, "\t-f: force reboot, don't call shutdown.\n");
	fprintf(stderr, "\t-i: shut down all network interfaces.\n");
    fprintf(stderr, "\t<restart mode>: \"recovery\", \"bootloader\", \"charger\"...\n");
	exit(1);
}

void alrm_handler(int sig)
{
	got_alrm = sig;
}

/*
 *	Set environment variables in the init process.
 */
int init_setenv(const char *name, const char *value)
{
	struct init_request	request;
	struct sigaction	sa;
	int			fd;
	int			nl, vl;

	memset(&request, 0, sizeof(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	vl = value ? strlen(value) : 0;

	if (nl + vl + 3 >= (int)sizeof(request.i.data))
		return -1;

	memcpy(request.i.data, name, nl);
	if (value) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, value, vl);
	}

        /*
	 *	Open the fifo and write the command.
         *	Make sure we don't hang on opening /run/initctl
	 */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = alrm_handler;
	sigaction(SIGALRM, &sa, NULL);
	got_alrm = 0;
	alarm(3);
	if ((fd = open(INIT_FIFO, O_WRONLY)) >= 0) {
		ssize_t p = 0;
		size_t s  = sizeof(request);
		void *ptr = &request;
		while (s > 0) {
			p = write(fd, ptr, s);
			if (p < 0) {
				if (errno == EINTR || errno == EAGAIN)
					continue;
				break;
			}
			ptr += p;
			s -= p;
		}
		close(fd);
		alarm(0);
		return 0;
	}
                                                                                
	fprintf(stderr, "%s: ", progname);
	if (got_alrm) {
		fprintf(stderr, "timeout opening/writing control channel %s\n",
			INIT_FIFO);
	} else {
		perror(INIT_FIFO);
	}
	return -1;
}


/*
 *	See if we were started directly from init.
 *	Get the runlevel from /var/run/utmp or the environment.
 */
int get_runlevel(void)
{
	struct utmp *ut;
	char *r;

	/*
	 *	First see if we were started directly from init.
	 */
	if (getenv("INIT_VERSION") && (r = getenv("RUNLEVEL")) != NULL)
		return *r;

	/*
	 *	Find runlevel in utmp.
	 */
	setutent();
	while ((ut = getutent()) != NULL) {
		if (ut->ut_type == RUN_LVL)
			return (ut->ut_pid & 255);
	}
	endutent();

	/* This should not happen but warn the user! */
	fprintf(stderr, "WARNING: could not determine runlevel"
		" - doing soft %s\n", progname);
	fprintf(stderr, "  (it's better to use shutdown instead of %s"
		" from the command line)\n", progname);

	return -1;
}

/*
 *	Switch to another runlevel.
 */
void do_shutdown(const char *mode)
{
	char *args[8];
	int i = 0;

    if( init_setenv("INIT_RESTART_MODE", mode) != 0 )
        return;
	args[i++] = "shutdown";
	args[i++] = "-r";
	args[i++] = "now";
	args[i++] = NULL;

	execv("/sbin/shutdown", args);
	execv("/etc/shutdown", args);
	execv("/bin/shutdown", args);

	perror("shutdown");
	exit(1);
}

/*
 *	Main program.
 *	Write a wtmp entry and reboot cq. halt.
 */
int main(int argc, char **argv)
{
	int do_sync = 1;
	int do_hard = 0;
	int do_ifdown = 0;
	int c;

	/*
	 *	Get flags
	 */
	while((c = getopt(argc, argv, ":ifn:")) != EOF) {
		switch(c) {
			case 'n':
				do_sync = 0;
				break;
			case 'f':
				do_hard = 1;
				break;
			case 'i':
				do_ifdown = 1;
				break;
			default:
				usage();
		}
	 }
	if (argc == optind) usage();

	if (geteuid() != 0) {
		fprintf(stderr, "%s: must be superuser.\n", progname);
		exit(1);
	}

	(void)chdir("/");

	if (!do_hard ) {
		/*
		 *	See if we are in runlevel 0 or 6.
		 */
		c = get_runlevel();
		if (c != '0' && c != '6')
			do_shutdown(argv[optind]);
	}

	if (do_sync) {
		sync();
		sleep(2);
	}

	if (do_ifdown)
		(void)ifdown();

    (void)hdflush();

    syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
            LINUX_REBOOT_CMD_RESTART2, argv[optind]);

	return 0;
}
