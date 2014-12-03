<?php

/**
 * @file
 * Handles the uploading of the PDF file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the tei file.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_cwrc_writer_tei_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('xml txt');
  $form = array();
  $file_size = array($upload_size * 1024 * 1024);
  $form['file'] = array(
    '#title' => t('TEI File'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      // Assume it's specified in MB.
      'file_validate_size' => $file_size,
    ),
  );
  $form['rng_schema'] = array(
    '#type' => 'textfield',
    '#title' => t('RNG Schema URL'),
    '#size' => 60,
    '#required' => FALSE,
  );
  $form['islandora_cwrc_writer_text_upload'] = array(
    '#type' => 'checkbox',
    '#title' => t("Add text file to this upload?"),
    '#default_value' => FALSE,
  );
  $form['text'] = array(
    '#title' => t('Plain text transcription'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select text file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b><br />This file is optional.', array('@size' => $upload_size, '@ext' => 'txt')),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('txt'),
      // Assume it's specified in MB.
      'file_validate_size' => $file_size,
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="islandora_cwrc_writer_text_upload"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['islandora_cwrc_writer_tn_upload'] = array(
    '#type' => 'checkbox',
    '#title' => t("Add optional thumbnail?"),
    '#default_value' => FALSE,
  );
  $form['tn'] = array(
    '#title' => t('Thumbnail for object'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select text file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong><br />This file is optional.', array('@size' => $upload_size, '@ext' => 'jpg, jpeg, png')),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array('jpg jpeg png'),
      // Assume it's specified in MB.
      'file_validate_size' => $file_size,
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="islandora_cwrc_writer_tn_upload"]' => array('checked' => TRUE),
      ),
    ),
  );
  return $form;
}

/**
 * Add's the uploaded file into the objects 'OBJ' datastream.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_cwrc_writer_tei_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $object = islandora_ingest_form_get_object($form_state);
  // Get the given datastream, create it if it does not exist.
  $get_datastream = function ($dsid) use ($object) {
    if (empty($object[$dsid])) {
      $datastream = $object->constructDatastream($dsid, 'M');
      $object->ingestDatastream($datastream);
      return $datastream;
    }
    return $object[$dsid];
  };
  // Update the 'OBJ' datastream.
  $tei_file = file_load($form_state['values']['file']);
  islandora_cwrc_writer_add_model_processing_instruction_to_file($tei_file->uri, $form_state['values']['rng_schema']);
  $ds = $get_datastream('OBJ');
  $ds->label = $tei_file->filename;
  $ds->mimetype = "text/xml";
  $ds->setContentFromFile(drupal_realpath($tei_file->uri), FALSE);
  // Optionally update 'FULL_TEXT' datastream.
  if (isset($form_state['values']['text']) && ($form_state['values']['text'] > 0)) {
    $text_file = file_load($form_state['values']['text']);
    $text_path = drupal_realpath($text_file->uri);
    $ds = $get_datastream('FULL_TEXT');
    $ds->setContentFromFile($text_path, FALSE);
    $ds->label = $text_file->filename;
    $ds->mimetype = $text_file->filemime;
  }
  // Optionally update 'TN' datastream.
  if ($form_state['values']['tn']) {
    $thumbnail_file = file_load($form_state['values']['tn']);
    islandora_scale_thumbnail($thumbnail_file, 200, 200);
    $ds = $get_datastream('FULL_TEXT');
    $ds->setContentFromFile(drupal_realpath($thumbnail_file->uri), FALSE);
    if ($ds->label != $thumbnail_file->filename) {
      $ds->label = $thumbnail_file->filename;
    }
    if ($ds->mimetype != $thumbnail_file->filemime) {
      $ds->mimetype = $thumbnail_file->filemime;
    }
  }
}
