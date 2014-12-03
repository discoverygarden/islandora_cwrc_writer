# Islandora CWRC-Writer [![Build Status](https://travis-ci.org/discoverygarden/islandora_cwrc_writer.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_cwrc_writer)

## Introduction

Provides a very minimal wrapper around the CWRC-Writer, so that it can be used in an islandora context.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Rest](https://github.com/discoverygarden/islandora_rest)
* [Libraries](https://www.drupal.org/project/libraries)
* [CWRC-Writer](https://github.com/discoverygarden/CWRC-Writer)
* [jQuery Update](https://www.drupal.org/project/jquery_update) Version 1.8

jQuery Update is not a hard requirement but is necessary if you want to use the 
templates in the documents dialog box. octokit.js which is used to fetch the 
templates uses the global jQuery rather than using require.js to fetch the 
CWRC-Writer specific template.

CWRC-Writer is expected to be installed here:

* sites/all/libraries/CWRC-Writer (libraries directory may need to be created)

So far we've only tested up to [commit 7f96f78e77](http://github.com/discoverygarden/CWRC-Writer/tree/7f96f78e774a2594ae8c2a3550549b01022dcc3f)

### Java Servlet Configuration

CWRC-Writer depends on a number of Java Servlets to be functional.

* [cwrc-validator](https://github.com/cwrc/cwrc-validator)

Follow the instructions on the page, but also *rename* the generated war to
validator.war. In most cases your tomcat should exist here
_/usr/local/fedora/tomcat_.

### Reverse proxy config:

We make the assumption that we (reverse) proxy VIAF, to fix the same-origin
issue.

For Apache, with Drupal running on the same box as Apache, a couple lines like:

```
ProxyPass /viaf http://www.viaf.org/viaf
ProxyPassReverse /viaf http://www.viaf.org/viaf
```

To be able to validate documents, we require that the validator.war is deployed
to your tomcat directory, and that you set up a (reverse) proxy so that the
CWRC-Writer can communicate with it.

```
ProxyPass /cwrc/services/validator/ http://localhost:8080/validator/
ProxyPassReverse /cwrc/services/validator/ http://localhost:8080/validator/
```

## To Do

* Look into integrating the [Geonames Service](http://github.com/cwrc/CWRC-Mapping-Timelines-Project/tree/master/geonames)
* Hook into existing islandora entities system for search and creation / deletion.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [discovery garden](https://github.com/discoverygarden)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.
The tests for this module will not run through Drupal’s UI. They will work using Drush, which works around Drupal’s batch API.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)

