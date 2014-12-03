<?php
/**
 * @file
 * Renders the CWRC-Writer.
 */
?>
<div id="cwrc_wrapper">
  <div id="cwrc_header" class="cwrc ui-layout-north">
    <h1><?php print $title; ?></h1>
    <?php print $header; ?>
  </div>
  <div class="cwrc ui-layout-west">
    <div id="westTabs" class="tabs">
      <?php print $western_tabs; ?>
      <div id="westTabsContent" class="ui-layout-content">
        <?php print $western_tabs_content; ?>
      </div>
    </div>
  </div>
  <div id="cwrc_main" class="ui-layout-center">
    <div class="ui-layout-center">
      <form method="post" action="">
        <textarea id="editor" name="editor" class="tinymce"></textarea>
      </form>
    </div>
    <div class="cwrc ui-layout-south">
      <div id="southTabs" class="tabs">
        <?php print $southern_tabs; ?>
        <div id="southTabsContent" class="ui-layout-content"></div>
      </div>
    </div>
  </div>
  <!-- The eastern panel is optional and may not be provided -->
  <?php if (isset($eastern_panel)): ?>
    <div class="ui-layout-east">
      <?php print $eastern_panel; ?>
    </div>
  <?php endif; ?>
</div>
