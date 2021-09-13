#!/sbin/openrc-run
# Copyright 2021 Neruthes
# Distributed under the terms of the GNU General Public License v2

SP_CONFIG="/etc/smartproxy.conf"

SP_PIDFILE="/run/smartproxy.pid"

depend() {
    need net
}

checkconfig() {
    if [ ! -f ${SP_CONFIG} ]; then
        ewarn "${SP_CONFIG} does not exist."
    fi

    SP_COMMAND="/usr/bin/smartproxy"
}

start() {
    checkconfig || return 1

    ebegin "Starting SmartProxy"
    start-stop-daemon --start --exec ${SP_COMMAND} \
    --user nobody --group nobody --background --make-pidfile --pidfile ${SP_PIDFILE} \
    -- --config ${SP_CONFIG} >/dev/null 2>&1 &
    eend $?
}

stop() {
    ebegin "Stopping SmartProxy"
    start-stop-daemon --stop \
    --user nobody --group nobody \
    --pidfile ${SP_PIDFILE}
    eend $?
}



