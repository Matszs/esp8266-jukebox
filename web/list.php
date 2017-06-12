<?php
include "db.php";

try {
	$result = $db->query('select id, title, url from soundslib');
	if (isset($_GET['json'])) {
		header('Content-Type: application/json');
		$j = array();
		foreach($result as $r) {
			$j[] = $r;
		}

		print json_encode($j);
	}
	elseif (isset($_GET['m3u'])) {
		header("Content-Type: audio/mpegurl");
		header("Content-Disposition: attachment; filename=playlist.m3u");
		echo "#EXTM3U\n\n";
		foreach($result as $r) {
			echo "#EXTINF:0, " . $r['title'] . "\n";
			echo $r['url'] . "\n\n";
		}
	}
	else {
		echo "<table border='1'><tr><th>ID</th><th>Titel</th><th>Url</th></tr>";
		foreach($result as $r) {
			echo "<tr><td>" . $r['id'] . "</td><td>" . $r['title'] . "</td><td>" . $r['url'] . "</td></tr>";
		}

		echo "</table>";
	}
}

catch(Exception $e) {
	print 'Exception : ' . $e->getMessage();
}

?>
