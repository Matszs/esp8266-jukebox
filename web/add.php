<?php
$soundsdir = "./sounds/";

if(isset($_POST['submit'])){

echo $_FILES['file']['name'];
	exec("mpg123 -w ".explode(".".$_FILES['file']['name'])[0].".wav ".$_FILES['file']['tmp_name']); 
	move_uploaded_file($_FILES['file']['tmp_name'],$soundsdir.$_FILES['file']['name']);

}


?>
<!DOCTYPE html>
<html>
<body>

<form action="add.php" method="post" enctype="multipart/form-data">
    Selecteer mp3 om te uploaden:
    <input type="file" name="file" id="file">
    <input type="submit" value="Mp3 uploaden" name="submit">
</form>

</body>
</html>

