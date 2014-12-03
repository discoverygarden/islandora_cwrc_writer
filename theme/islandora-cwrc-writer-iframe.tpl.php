<?php
/**
 * @file
 * Returns the a full HTML document for rendering the CWRC-Writer it's meant
 * only for use within an iFrame.
 *
 * Complete documentation for this file is available online.
 * @see https://drupal.org/node/1728208
 */
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
  <head>
    <meta http-equiv="Content-type" content="text/html;charset=UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=Edge" />
    <title>CWRC-Writer</title>
    <?php print $scripts; ?>
    <!--
    We render only require.js inline as it requires data-main to be set
    and we can't do that with the normal drupal_add_js() functions. And we have
    to do this cause of funky hardcoded pathing in the CWRC-Writer.
    -->
    <script type="text/javascript" data-main="<?php print "/{$cwrc['require']['config']}"; ?>" src="<?php print "/{$cwrc['require']['js']}"; ?>"></script>
    <?php print $styles; ?>
  </head>
  <body>
    <?php print $content; ?>
  </body>
</html>
