<?php

include "db.php";

try{
$result = $db->query('select id, title, url from soundslib');

echo "<table border='1'>";


foreach($result as $r)
{
    echo "<td>Id: ".$r['id']."</td><td>Titel: ".$r['title']."</td><td>Url: ".$r['url']."</td>";
}


echo "</table>";




}
catch(Exception $e)
{
    print 'Exception : '.$e->getMessage();
}
?>
