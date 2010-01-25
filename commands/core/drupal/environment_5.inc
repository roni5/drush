<?php
// $Id$
/**
 * @file
 *   Specific functions for a drupal 5 environment.
 *   drush_include_engine() magically includes either this file
 *   or environment_X.inc depending on which version of drupal drush
 *   is called from.
 */

/**
 * Get complete information for all available modules.
 *
 * @return
 *   An array containing module info for all available modules.
 */
function drush_get_modules() {
  $modules = module_rebuild_cache();
  // In Drupal 5, system_modules() returns NULL for the dependency list of the module if there are no dependencies.
  // We will override this to be an empty array instead to be compatible to Drupal 6 and 7
  foreach ($modules as $module) {
    if (empty($module->info['dependencies'])) {
      $module->info['dependencies'] = array();
    }
    if (empty($module->info['dependents'])) {
      $module->info['dependents'] = array();
    }
  }
  return $modules;
}

/**
 * Get complete information for all available themes.
 *
 * @return
 *   An array containing theme info for all available themes.
 */
function drush_get_themes() {
  return list_themes();
}

/**
 * Return dependencies and its status for modules.
 *
 * @param $modules
 *   Array of module names
 * @param $module_info
 *   Drupal 'files' array for modules as returned by drush_get_modules().
 * @return
 *   Array with dependencies and status for $modules
 */
function drush_check_module_dependencies($modules, $module_info) {
  $status = array();
  foreach ($modules as $key => $module) {
    $dependencies = $module_info[$module]->info['dependencies'];
    $unmet_dependencies = array_diff($dependencies, array_keys($module_info));
    if (!empty($unmet_dependencies)) {
      $status[$key]['error'] = array(
          'code' => 'DRUSH_PM_ENABLE_DEPENDENCY_NOT_FOUND',
          'message' => dt('Module !module cannot be enabled because it depends on the following modules which could not be found: !unmet_dependencies', array('!module' => $module, '!unmet_dependencies' => implode(',', $unmet_dependencies))) 
      );
    }
    else {
      $status[$key]['dependencies'] = $dependencies;
    }
  }

  return $status;
}

/**
 * Return dependents of modules.
 *
 * @param $modules
 *   Array of module names
 * @param $module_info
 *   Drupal 'files' array for modules as returned by drush_get_modules().
 * @return
 *   Array with dependents for each one of $modules
 */
function drush_module_dependents($modules, $module_info) {
  $dependents = array();
  foreach ($modules as $module) {
    $dependents = array_merge($dependents, $module_info[$module]->info['dependents']);
  }

  return array_unique($dependents);
}

/**
 * Enable a list of modules. It is assumed the list contains all the dependencies not already enabled.
 * 
 * @param $modules
 *   Array of module names
 */
function drush_module_enable($modules) {
  // In Drupal 5, drupal_install_modules() only installs new modules,
  // and does not enable previously installed and disabled modules.
  $install_modules = array();
  $enable_modules = array();

  foreach ($modules as $module) {
    if (drupal_get_installed_schema_version($module) == SCHEMA_UNINSTALLED) {
      $install_modules[] = $module;
    }
    else {
      $enable_modules[] = $module;
    }
  }
  drupal_install_modules($install_modules);
  module_enable($enable_modules);
}

/**
 * Disable a list of modules. It is assumed the list contains all dependents not already disabled.
 *
 * @param $modules
 *   Array of module names
 */
function drush_module_disable($modules) {
  module_disable($modules);
}

/**
 * Uninstall a list of modules.
 *
 * @param $modules
 *   Array of module names
 */
function drush_module_uninstall($modules) {
  require_once drush_get_context('DRUSH_DRUPAL_ROOT') . '/includes/install.inc';
  foreach ($modules as $module) {
    drupal_uninstall_module($module);
  }
}

/**
 * Submit the system modules form.
 *
 * The modules should already be fully enabled/disabled before calling this 
 * function. Calling this function just makes sure any activities triggered by
 * the form submit (such as admin_role) are completed.
 */
function drush_system_modules_form_submit($active_modules) {
  require_once('./'. drupal_get_path('module', 'system') .'/system.module');
  $form_state = array('values' => array('status' => $active_modules));
  drupal_execute('system_modules', $form_state);
}
