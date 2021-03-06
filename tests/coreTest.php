<?php

/*
 * @file
 *   Tests for core commands.
 */
class coreCase extends Drush_CommandTestCase {

  /*
   * Test standalone php-script scripts. Assure that script args and options work.
   */
  public function testStandaloneScript() {
    $this->drush('version', array('drush_version'), array('pipe' => NULL));
    $standard = $this->getOutput();

    // Write out a hellounish.script into the sandbox. The correct /path/to/drush
    // is in the shebang line.
    $filename = 'hellounish.script';
    $data = '#!/usr/bin/env [PATH-TO-DRUSH]

$arg = drush_shift();
drush_invoke("version", $arg);
';
    $data = str_replace('[PATH-TO-DRUSH]', UNISH_DRUSH, $data);
    $script = UNISH_SANDBOX . '/' . $filename;
    file_put_contents($script, $data);
    chmod($script, 0755);
    $this->execute("$script drush_version --pipe");
    $standalone = $this->getOutput();
    $this->assertEquals($standard, $standalone);
  }

  function testDrupalDirectory() {
    $this->setUpDrupal(1, TRUE);
    $root = $this->webroot();
    $options = array(
      'root' => $root,
      'uri' => key($this->sites),
      'verbose' => NULL,
      'skip' => NULL, // No FirePHP
      'yes' => NULL,
      'cache' => NULL,
      'invoke' => NULL, // invoke from script: do not verify options
    );
    $this->drush('pm-download', array('devel'), $options);
    $this->drush('pm-enable', array('devel', 'menu'), $options);

    $this->drush('drupal-directory', array('devel'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/all/modules/devel', $output);

    $this->drush('drupal-directory', array('%files'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/dev/files', $output);

    $this->drush('drupal-directory', array('%modules'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/all/modules', $output);
  }

  function testCoreRequirements() {
    $this->setUpDrupal(1, TRUE);
    $root = $this->webroot();
    $options = array(
      'root' => $root,
      'uri' => key($this->sites),
      'pipe' => NULL,
      'ignore' => 'cron,http requests,update_core', // no network access when running in tests, so ignore these
      'invoke' => NULL, // invoke from script: do not verify options
    );
    // Verify that there are no severity 2 items in the status report
    $this->drush('core-requirements', array(), $options + array('severity' => '2'));
    $output = $this->getOutput();
    $this->assertEquals('', $output);
    $this->drush('core-requirements', array(), $options);
    $output = $this->getOutput();
    $expected="database_system: -1
database_system_version: -1
drupal: -1
file system: -1
install_profile: -1
node_access: -1
php: -1
php_extensions: -1
php_memory_limit: -1
php_register_globals: -1
settings.php: -1
unicode: 0
update: 0
update access: -1
update status: -1
webserver: -1";
    $this->assertEquals($expected, trim($output));
  }
}
