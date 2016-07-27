delete from T_sessions where strftime('%s','2016-04-27 15:01:24')-strftime('%s',T_sessions.LastUpdateTime) > 30;
