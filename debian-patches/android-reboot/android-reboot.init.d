#! /bin/sh
### BEGIN INIT INFO
# Provides:		android-reboot
# Required-Start:	
# Required-Stop:   	kexec reboot
# X-Stop-After:		umountroot
# Default-Start:
# Default-Stop:		6
# Short-Description: Reboot with restart mode parameter
# Description:
### END INIT INFO

PATH=/sbin:/bin

. /lib/lsb/init-functions

do_stop () {
	[ -n "$INIT_RESTART_MODE" ] || exit 0
	test -x /sbin/android-reboot || exit 0

	log_action_msg "Will now restart $INIT_RESTART_MODE"
	/sbin/android-reboot -f -i "$INIT_RESTART_MODE"
}

case "$1" in
  start)
	# No-op
	;;
  restart|reload|force-reload)
	echo "Error: argument '$1' not supported" >&2
	exit 3
	;;
  stop)
	do_stop
	;;
  *)
	echo "Usage: $0 start|stop" >&2
	exit 3
	;;
esac
exit 0
