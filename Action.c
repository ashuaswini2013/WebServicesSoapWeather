/**
* Loadrunner script to perf test SOAP based Webservice.
* This uses weather wsdl to fetch the weather report of specified zipcode.
* 
* Fuctions covered in this,
* web_service_call
* lr_xml_get_values
* lr_save_param_regexp
* lr_output_message
* lr_eval_string
*/
Action()
{

	char *zipcodeOutputStr;
	int matchCt;
	int i, summaryCount, tempCount;
	char buf[64];
	
	// Fetch coordinates data of specific zipcode
	web_service_call( "StepName=LatLonListZipCode_101",
		"SOAPMethod=ndfdXML|ndfdXMLPort|LatLonListZipCode",
		"ResponseParam=zipCodeRespone",
		"Service=ndfdXML",
		"ExpectedResponse=SoapResult",
		"Snapshot=t1648935948.inf",
		BEGIN_ARGUMENTS,
		"zipCodeList={zipcode}",
		END_ARGUMENTS,
		BEGIN_RESULT,
		"LatLonListZipCodeResult=Param_LatLonListZipCodeResult",
		END_RESULT,
		LAST);

	// Parase xml data to fetch output data from response
     lr_xml_get_values("XML={zipCodeRespone}",
          "ValueParam=zipCodeOutputParam",
          "Query=//listLatLonOut",
          LAST );

	// Saving the param to C variable as its needed for lr_save_param_regexp
	zipcodeOutputStr = lr_eval_string("{zipCodeOutputParam}");

	// Option 1: Reading coordinates from the xml
	lr_xml_get_values("XML={zipCodeOutputParam}",
          "ValueParam=coordinatesParam",
          "Query=//latLonList",
          LAST );
	
	// Option 2: Read latitude and longitude data from xml and save it in a param
	//Save latitude value to param
	lr_save_param_regexp(
	        zipcodeOutputStr,
	        strlen(zipcodeOutputStr),
	        "Ordinal=1",
	        "RegExp=latLonList>(.*?),.*?<",
	        "ResultParam=latitudeParam",
	        LAST
	    );
	//Save longitude value to param
	lr_save_param_regexp(
	        zipcodeOutputStr,
	        strlen(zipcodeOutputStr),
	        "Ordinal=1",
	        "RegExp=latLonList>.*?,(.*?)<",
	        "ResultParam=longitudeParam",
	        LAST
	    );

	lr_output_message("%s : %s ",lr_eval_string("{latitudeParam}"),lr_eval_string("{longitudeParam}"));
	
	//Request webservice to fetch weather forcast by day for given coordinates and date
	web_service_call( "StepName=NDFDgenByDay_101",
		"SOAPMethod=ndfdXML|ndfdXMLPort|NDFDgenByDay",
		"ResponseParam=weatherByDayResponse",
		"Service=ndfdXML",
		"ExpectedResponse=SoapResult",
		"Snapshot=t1648951099.inf",
		BEGIN_ARGUMENTS,
		"latitude={latitudeParam}",
		"longitude={longitudeParam}",
		"startDate={currentDate}",
		"numDays=5",
		"Unit=e",
		"format=24 hourly",
		END_ARGUMENTS,
		BEGIN_RESULT,
		"NDFDgenByDayResult=Param_NDFDgenByDayResult",
		END_RESULT,
		LAST);
	
	// Parse xml output data and save it to param
    lr_xml_get_values("XML={weatherByDayResponse}",
          "ValueParam=dwmlByDayOutParam",
          "Query=//dwmlByDayOut",
          LAST );
	
	// Parse xml and read wetherSummary of each day from dwmlByDayOutParam 
	summaryCount= lr_xml_get_values("XML={dwmlByDayOutParam}",
          "ValueParam=weatherSummaryDay",
          "Query=//weather-conditions/@weather-summary",
          "SelectAll=yes",
          LAST );
	
	// Parse xml and read maximum tempature of each day from dwmlByDayOutParam 
	tempCount= lr_xml_get_values("XML={dwmlByDayOutParam}",
          "ValueParam=weatherTemperature",
          "Query=//temperature[@type='maximum']/value",
          "SelectAll=yes",
          LAST );
	
	// Iterate over the summary count and print output in readable way to console
	lr_output_message("Zip Code: %s",lr_eval_string("{zipcode}"));
	for ( i = 0; i < summaryCount; i++) { /* Print multiple values of OutputParam */
		sprintf (buf, "Day %d : {weatherTemperature_%d} {weatherSummaryDay_%d}", i+1, i+1, i+1);
        lr_output_message(lr_eval_string(buf));
    }
	
	return 0;
}
