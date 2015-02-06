/*jshint nonew: false, browser: true */
/*global jQuery, Drupal*/
var writer = null;
/**
 * CWRC-Writer global callback used to configure the CWRC-Writer.
 *
 * @param $
 * @param Writer
 * @param Delegator
 */
function cwrcWriterInit($, Writer, Delegator) {
  'use strict';
  var config = Drupal.settings.CWRCWriter;

  function doResize() {
    var uiHeight = $('#' + writer.editor.id + '_tbl tr.mceFirst').outerHeight() + 2;
    $('#' + writer.editor.id + '_ifr').height($(window).height() - uiHeight);
  }

  function setupLayoutAndModules(w, EntitiesList, StructureTree, Validation) {
    w.layout = $('#cwrc_wrapper').layout({
      defaults: {
        maskIframesOnResize: true,
        resizable: true,
        slidable: false
      },
      west: {
        size: 'auto',
        minSize: 225,
        onresize: function (region, pane, state, options) {
          var tabsHeight = $('#westTabs').find('> ul').outerHeight();
          $('#westTabsContent').height(state.layoutHeight - tabsHeight);
        }
      }
    });

    new StructureTree({writer: w, parentId: 'westTabsContent'});
    new EntitiesList({writer: w, parentId: 'westTabsContent'});
    new Validation({writer: w, parentId: 'southTabsContent'});

    $('#westTabs').tabs({
      active: 1,
      activate: function (event, ui) {
        $.layout.callbacks.resizeTabLayout(event, ui);
      },
      create: function (event, ui) {
        $('#westTabs').parent().find('.ui-corner-all').removeClass('ui-corner-all');
      }
    });

    setTimeout(function () {
      w.layout.resizeAll(); // now that the editor is loaded, set proper sizing
    }, 250);
  }

  config.id = 'editor';
  config.delegator = Delegator;
  config.mode = 'xml';
  config.buttons1 = 'schematags,editTag,removeTag,|,addperson,addplace,adddate,addorg,addcitation,addtitle,addcorrection,addkeyword,addlink';
  writer = new Writer(config);
  writer.event('writerInitialized').subscribe(doResize);
  $(window).on('resize', doResize);

  writer.event('writerInitialized').subscribe(function (writer) {
    // load modules then do the setup
    require(['modules/entitiesList', 'modules/structureTree', 'modules/validation'], function (EntitiesList, StructureTree, Validation) {
      setupLayoutAndModules(writer, EntitiesList, StructureTree, Validation);
    });
  });
}
