<?php

/**
 * @file
 * Endpoints page callbacks for the Cinema Promotions module.
 */
 
 /**
  * Function to render the stored data
  */
function cinema_promotions_overview() {
 	// Headers of the table
	$header = array();
 	$header[] = array('data' => t('Name'), 'field' => 'name');
 	$header[] = array('data' => t('Description'), 'field' => 'description');
 	$header[] = array('data' => t('Code'), 'field' => 'code');
 	$header[] = array('data' => t('Operations'));
 	
 	// Query to get the values of the table
 	$result = _get_promotions_from_db();

 	// Rows of the table
 	$rows = array();
 	
 	foreach ($result as $data) {
	    $row = array();
	    $row['data']['name'] = $data['name'];
	    $row['data']['description'] = $data['description']; 
	    $row['data']['code'] = $data['code']; 
	    
	    $operations = array();
	    $operations['edit'] = array(
	      'title' => t('edit'),
	      'href' => CINEMA_PROMOTIONS_DATA_PATH . '/edit/'. $data['pid'],
	    );
	    $operations['delete'] = array(
	      'title' => t('delete'),
	      'href' => CINEMA_PROMOTIONS_DATA_PATH . '/delete/'. $data['pid'],
	    );
	    
	    $row['data']['operations'] = array(
	      'data' => array(
	        '#theme' => 'links',
	        '#links' => $operations,
	        '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
	      ),
	    );
	    
	    $rows[] = $row;
 	}
 	
 	// Configuration for the table
 	$build['path_table'] = array(
    	'#theme' => 'table',
    	'#header' => $header,
    	'#rows' => $rows,
    	'#empty' => t('No promotions available. <a href="@link">Add Promotion</a>.', 
    		array('@link' => url(CINEMA_PROMOTIONS_DATA_PATH . '/add'))),
  	);

  	return $build;
}
 
/**
 * Form builder 
 * Form for adding and editing the value for Cinema Promotions.
 *
 * @see cinema_promotions_form_submit()
 * @see cinema_promotions_form_validate()
 */
function cinema_promotions_form($form, &$form_state, $pid = null) {
	// Check if the operation is add or edit
	if ($pid) {
		$value = _get_promotion_from_db($pid);
	}	
	
	// Promotion's id
	$form['pid'] = array(
		'#type' => 'hidden', 
		'#value' => $pid,
	);
		
	// Promotion's name
	$form['name'] = array(
    	'#type' => 'textfield',
    	'#size' => '128',
    	'#title' => t('Promotion name'),
	    '#default_value' => isset($value) ? $value['name'] : '',
	    '#description' => t('Name to include as the promotion identifier. <br/>' . 
	    					'For example: Premier Avengers Promotion'),
	    '#required' => TRUE,
	);
	
	// Promotions's description
	$form['description'] = array(
		'#type' => 'textarea',
    	'#title' => 'Description',
	    '#default_value' => isset($value) ? $value['description'] : '',
	    '#description' => t('Description of the promotion that will be displayed to the user.'),
	    '#required' => TRUE,
	);
	
	// Promotion's code
	$form['code'] = array(
		'#type' => 'textfield',
    	'#size' => '64',
    	'#title' => t('Code'),
	    '#default_value' => isset($value) ? $value['code'] : '',
	    '#description' => t('Code that will be used to redempt the promotion.'),
	    '#element_validate' => array('cinema_promotions_form_validate'),
	    '#required' => TRUE,
	);

	// Form submit button
  	$form['submit'] = array(
	    '#type' => 'submit',
	    '#value' => t('Save'),
  	);

  	return $form;
}

/**
 * Function that validates the promotion form when it is submited.
 */
function cinema_promotions_form_validate($element, $form_state) {
	if (isset($element['#name'])) {
		// Check if the element is the code of the promotion
		if ($element['#name'] == 'code') {
			// Check that the code has only numbers
			if (!is_numeric($form_state['values']['code'])) {
				form_set_error($element['#name'], 
	  				t('The code should have only numbers.'));
			}
		}
	}
}

/**
 * Function to insert or update the promotion in the database when form submits.
 */
function cinema_promotions_form_submit($form, &$form_state) {
  	try {
  		// Check if the operation is insert or update
  		if ($form_state['values']['pid'] && $form_state['values']['pid'] != '') {
  			// Update the promotion
  			db_update('cinema_promotions_data')
		    	->fields(array(
		      		'name' => $form_state['values']['name'],
		      		'description' => $form_state['values']['description'],
		      		'code' => $form_state['values']['code'],
				))
				->condition('pid', $form_state['values']['pid'])
				->execute();
  		} else {
  			// Insert a new promotion
			db_insert('cinema_promotions_data')
		    	->fields(array(
		      		'name' => $form_state['values']['name'],
		      		'description' => $form_state['values']['description'],
		      		'code' => $form_state['values']['code'],
			))->execute();
  		}
  		
		drupal_set_message(t('The promotion was successfully saved.'));
		$form_state['redirect'] = CINEMA_PROMOTIONS_DATA_PATH;
	} catch (Exception $e) {
		drupal_set_message(t('There was an error saving the promotion. Please try again.'), 'error');
		
		// Check where to redirect the user
		if ($form_state['values']['pid'] && $form_state['values']['pid'] != '') {
			$form_state['redirect'] = CINEMA_PROMOTIONS_DATA_PATH . '/edit/' . $form_state['values']['pid'];
		} else {
	  		$form_state['redirect'] = CINEMA_PROMOTIONS_DATA_PATH . '/add';
		}
  	}
}

/**
 * Form builder
 * Form for deleting a value for Cinema Promotions.
 *
 * @see cinema_promotions_delete_form_submit()
 */
function cinema_promotions_delete_form($form, &$form_state, $pid = null) {
	if ($pid) {
		$value = _get_promotion_from_db($pid);
	}	

	$form['pid'] = array(
		'#type' => 'hidden', 
		'#value' => $pid);
  	
  	return confirm_form(
	  	$form, 
	  	t('Are you sure you want to delete the Cinema Promotion "%promotion"?', 
	  		array('%promotion' => $value['name'])),
	  	CINEMA_PROMOTIONS_DATA_PATH
	);
}

/**
 * Function to delete the promotion from the database.
 */
function cinema_promotions_delete_form_submit($form, &$form_state) {
	try {
		// Delete the promotion
		db_delete('cinema_promotions_data')
			->condition('pid', $form_state['values']['pid'])
	    	->execute();
	  		
		drupal_set_message(t('The promotion was successfully deleted.'));
		$form_state['redirect'] = CINEMA_PROMOTIONS_DATA_PATH;
	} catch (Exception $e) {
  		drupal_set_message(t('There was an error deleting the promotino. Please try again.'), 'error');
  		$form_state['redirect'] = CINEMA_PROMOTIONS_DATA_PATH . '/delete/' . $form_state['values']['pid'];
  	}
}

/**
 * Form builder 
 * Form for general Cinema Promotions configuration.
 *
 * @see cinema_promotions_configuration_form_submit()
 * @see cinema_promotions_configuration_form_validate()
 */
function cinema_promotions_configuration_form($form, &$form_state) {
	// Promotions random factor
	$form['random_factor'] = array(
    	'#type' => 'textfield',
    	'#size' => '8',
    	'#title' => t('Promotions random factor'),
	    '#default_value' => variable_get('cinema_promotions_random_factor', 0),
	    '#description' => t('Random factor percentage used to calculate if the promotion should be displayed to the user.'),
	    '#required' => TRUE,
	    '#field_suffix' => '%',
	    '#element_validate' => array('cinema_promotions_configuration_form_validate'),
	);

	// Form submit button
  	$form['submit'] = array(
	    '#type' => 'submit',
	    '#value' => t('Save'),
  	);

	return $form;
}

/**
 * Function that validates the configuration form when it is submited.
 */
function cinema_promotions_configuration_form_validate($element, $form_state) {
	if (isset($element['#name'])) {
		// Check if the element is the code of the promotion
		if ($element['#name'] == 'random_factor') {
			// Check that the code has only numbers
			if (!is_numeric($form_state['values']['random_factor'])) {
				form_set_error($element['#name'], 
	  				t('The random factor should have only numbers.'));
			}
		}
	}
}

/**
 * Function to save the promotion configurations to the database.
 */
function cinema_promotions_configuration_form_submit($form, &$form_state) {
	variable_set('cinema_promotions_random_factor', $form_state['values']['random_factor']);
	drupal_set_message(t('The configuration was saved successfully.'));
	$form_state['redirect'] = CINEMA_PROMOTIONS_CONFIG_PATH;
}

/**
 * Private function to retrieve the endpoints from the database
 */
function _get_promotions_from_db() {
	return db_select('cinema_promotions_data', 'p')
				->fields('p')
				->execute()
				->fetchAllAssoc('pid', PDO::FETCH_ASSOC);	
}

/**
 * Private function to get a promotion from the database
 */
function _get_promotion_from_db($pid) {
	return db_select('cinema_promotions_data', 'p')
		    ->fields('p')
		    ->condition('pid', $pid, '=')
		    ->execute()
		    ->fetchAssoc();
}
