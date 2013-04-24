#! /bin/sh
# /etc/init.d/rgbclock

### BEGIN INIT INFO
# Provides:        rgbclock
# Required-Start:  $remote_fs $syslog
# Required-Stop:   $remote_fs $syslog
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6 
# Short-Descripti on: Start RGB Clock daemon
### END INIT INFO



PATH=/sbin:/bin:/usr/sbin:/usr/bin

. /lib/lsb/init-functions

DAEMON=/usr/bin/rgbclock/rgbclock
PIDFILE=/var/run/rgbclock.pid
LOGDIR=/var/log/rgbclock

test -x $DAEMON || exit 5


LOCKFILE=/var/lock/rgbclockdate

lock_rgbclockdate() {
	if [ -x /usr/bin/lockfile-create ]; then
		lockfile-create $LOCKFILE
		lockfile-touch $LOCKFILE &
		LOCKTOUCHPID="$!"
	fi
}

unlock_rgbclockdate() {
 	if [ -x /usr/bin/lockfile-create ] ; then
		kill $LOCKTOUCHPID
		lockfile-remove $LOCKFILE
	fi
}


case $1 in
	start)
		log_daemon_msg "Starting RGB Clock" "rgbclock"

#		lock_rgbclockdate
  		start-stop-daemon --start --quiet --oknodo --pidfile $PIDFILE --startas $DAEMON -- -daemon true -pidfile $PIDFILE -log_dir $LOGDIR
		status=$?
#		unlock_rgbclockdate
		log_end_msg $status
  		;;
	stop)
		log_daemon_msg "Stopping RGB Clock" "rgbclock"
  		start-stop-daemon --stop --quiet --oknodo --pidfile $PIDFILE
		log_end_msg $?
		rm -f $PIDFILE
  		;;
	restart|force-reload)
		$0 stop && sleep 2 && $0 start
  		;;
	status)
                status_of_proc -p $PIDFILE $DAEMON  "RGB Clock"  
		;;
	*)
		echo "Usage: $0 {start|stop|restart|force-reload|status}"
		exit 2
		;;
esac
