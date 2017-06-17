<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

include "db.php";

$soundsdir = "./sounds/";

if (isset($_POST['submit'])) {
    $title = $_POST['title'];
    $execmpg123 = 'mpg123 -w /var/www/esp8266-jukebox/web/sounds/' . strtolower($title) . '.wav --8bit --rate 8000 --mono ' . $_FILES['file']['tmp_name'];

    //echo $execmpg123;
    exec($execmpg123);


    $qry = $db->prepare('INSERT INTO soundslib (title, url) VALUES (?, ?)');
    $output = $qry->execute(array(strtolower($title),"http://jukebox.derfu.nl/sounds/".strtolower($title).".wav"));

    print_r($qry->errorInfo());

    echo $output;

    var_dump($output);
}
?>
<!DOCTYPE html>
<html>
<body>

<form action="add.php" method="post" enctype="multipart/form-data">
    Selecteer mp3 om te uploaden:
    <input type="file" name="file" id="file">
    <input type="text" name="title" id="title">
    <input type="submit" value="Mp3 uploaden" name="submit">
</form>

</body>
</html>
