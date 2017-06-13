<?php

$db = new PDO('sqlite:/var/www/esp8266-jukebox/web/db/sounds.db');
$db->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
?>
