<?php

/**
 * @file
 * Menu callbacks and hooks that are related to fetching / saving documents.
 */

/**
 * Endpoint for GET, PUT, DELETE requests from the CWRC-Writer for documents.
 *
 * @todo For now assume the datastream is always CWRC, but we should have a hook
 * that checks the Content Model of the object then hooks other modules to say
 * what datastream should be modified. Or we could make the editor aware of what
 * datastream is being edited, it could be used as the document id, like
 * #pid/dsid.
 *
 * @param string $pid
 *   The PID of the object to fetch / modify.
 * @param string $dsid
 *   The DSID containing the document, right now assumed to be CWRC.
 */
function islandora_cwrc_writer_document($pid, $dsid = 'CWRC') {
  module_load_include('inc', 'islandora_rest', 'includes/utilities');
  module_load_include('inc', 'islandora_rest', 'includes/datastream');
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $request_method = islandora_rest_get_request_method();
  try {
    // Fetch the requested resource. Throws 404, if the resource is not found.
    $document = islandora_rest_get_requested_resource($pid, $dsid);
    // Check Drupal Permissions. Throws 401, 403, if access is denied.
    islandora_rest_require_access('datastream', $request_method, $document);
    // Generate the Response, we don't implement POST / DELETE as we don't
    // currently allow the user to make new documents or remove them.
    switch ($request_method) {
      case 'GET':
        // The response from get_requested_resource should have processing
        // instructions appended already, so no need to add them.
        try {
          $response = islandora_cwrc_writer_style_document($document);
        }
        catch (InvalidArgumentException $e) {
          // Just use unstyled content if there's a problem.
          $response = $document->content;
        }
        break;

      case 'PUT':
        // The default islandora rest module PUT response won't work in this
        // case as CWRC-Writer does not do it as a POST (Means it won't work in
        // IE), but I don't think they care about that. So we have to implement
        // actual PUT requests now. I'm not sure if this function is ready for
        // show-time, perhaps we'll integrated it with Islandora Rest at a later
        // time.
        $response = islandora_cwrc_writer_update_document($document);
        break;

      default:
        throw new Exception('Not Implemented', 500);
    }
  }
  catch (Exception $e) {
    drupal_add_http_header('Status', $e->getCode());
    // CWRC-Writer currently ignores the response aside from the status code,
    // when an error occurs.
    $response = array('message' => $e->getMessage());
  }
  $encode = is_array($response) || is_object($response);
  if ($encode) {
    // Responses that don't generate objects or arrays, must set their own
    // content types. Not calling drupal_json_output because we are setting
    // encoding.
    drupal_add_http_header('Content-Type', 'application/json; utf-8');
    drupal_json_output($response);
  }
  else {
    echo $response;
  }
  drupal_exit();
}

/**
 * Responds to the PUT request to update the document datastream.
 *
 * Modifies and existing datastream, with the uploaded content, also adds a
 * processing instruction for the chosen schema.
 *
 * @param AbstractDatastream $datastream
 *   The datastream to update with the put contents.
 *
 * @return array
 *   JSON indicating that the PUT was successful.
 */
function islandora_cwrc_writer_update_document(AbstractDatastream $datastream) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/document');

  $put_data = array();
  parse_str(file_get_contents("php://input"), $put_data);
  islandora_cwrc_writer_set_document_schema(
    $datastream->parent,
    islandora_object_load($put_data['schema'])
  );

  $temp_name = drupal_tempnam(file_default_scheme() . '://', 'file');
  $temp = fopen($temp_name, 'w');
  fwrite($temp, $put_data['doc']);
  fclose($temp);
  // We must remove the xml-model processing instruction from the file, it will
  // cause problems in other modules since it is not well supported by PHP
  // version of libxml.
  islandora_cwrc_writer_remove_model_processing_instruction_from_file($temp_name);
  $datastream->setContentFromFile($temp_name);
  drupal_unlink($temp_name);
  return array('success' => TRUE);
}


/**
 * Adds styling to a CWRC document from its schema.
 *
 * @param AbstractDatastream $datastream
 *   The CWRC datastream to style.
 *
 * @throws InvalidArgumentException
 *
 * @return mixed
 *   The styled document as a string.
 */
function islandora_cwrc_writer_style_document(AbstractDatastream $datastream) {
  $object = $datastream->parent;

  if ($datastream->id == 'CWRC' && in_array('cwrc:documentCModel', $object->models)) {
    // Get the schema object. If there's a problem, bail.
    $schema_pid = $object->relationships->get('http://cwrc.ca/ns/cw#', 'hasSchema');
    if ($schema_pid) {
      $schema_object = islandora_object_load($schema_pid[0]['object']['value']);
      if (!$schema_object) {
        watchdog('islandora_cwrc_writer',
          'The hasSchema relationship on !object pointed to an object (!schema_object) that could not be loaded.',
          array(
            '!object' => $object->id,
            '!schema_object' => $schema_pid,
          ),
          WATCHDOG_WARNING
        );
        throw new InvalidArgumentException("Can not load schema object $schema_pid for {$object->id}");
      }

      $accessibles = islandora_cwrc_writer_get_rng_css_accessibility($schema_object);
      if ($accessibles) {
        // Get and possibly modify the original content.
        if ($datastream->controlGroup == 'X' || 'M') {
          global $base_url;
          $cwrc = $datastream->content;
          module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');

          // Attempt to add the two datastreams.
          foreach ($accessibles as $accessible) {
            $url = $schema_object[$accessible]->controlGroup === 'X' || $schema_object[$accessible]->controlGroup === 'M' ?
              "$base_url/islandora/object/{$schema_object->id}/datastream/$accessible/view" :
              $schema_object[$accessible]->url;
            if ($accessible == 'SCHEMA') {
              $cwrc = islandora_cwrc_writer_add_model_processing_instruction_to_string($cwrc, $url);
            }
            else {
              $cwrc = islandora_cwrc_writer_add_stylesheet_processing_instruction_to_string($cwrc, $url, 'text/css');
            }
            // If something goes wrong, bail and return false.
            if (!$cwrc) {
              watchdog('islandora_cwrc_writer',
                'The !accessible datastream on !pid could not be added to the CWRC datastream as it created malformed XML.',
                array(
                  '!accessible' => $accessible,
                  '!pid' => $schema_pid,
                ),
                WATCHDOG_WARNING
              );
              throw new InvalidArgumentException("Can not add $accessible datastream on {$schema_object->id} to the CWRC datastream of {$object->id}.");
            }
          }
          return $cwrc;
        }
      }
    }
  }

  // Default in all other cases.
  throw new InvalidArgumentException("Provided datastream is not a CWRC datastream or its parent is not a cwrc:documentCModel.");
}

/**
 * Helper to only check the accessibility of these datastreams once.
 *
 * @param AbstractObject $object
 *   The object the datastreams should exist on.
 *
 * @return array
 *   An array containing the IDs of accessible datastreams.
 */
function islandora_cwrc_writer_get_rng_css_accessibility($object) {
  global $user;
  $results = array();
  foreach (array('CSS', 'SCHEMA') as $datastream) {
    if (isset($object[$datastream]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$datastream], $user)) {
      $results[] = $datastream;
    }
  }
  return $results;
}
