var CURRENT_VERSION = 2;

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function fireWeather(url)
{
        xhrRequest(url, 'GET', 
        function(responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);
      
            // Temperature in Kelvin requires adjustment
            var celsius = true;
            if (localStorage.temp_unit)
                celsius = localStorage.temp_unit == "0";
            
            localStorage.kelvin = json.main.temp;
            var temperature = Math.round(celsius ? json.main.temp - 273.15 : json.main.temp * 9 / 5 - 459.67);
            console.log('Temperature is ' + temperature);
      
            // Conditions
            var iconstr = json.weather[0].icon;
            var icon = 0;
            if (iconstr == "01d") icon = 0;
            if (iconstr == "01n") icon = 1;
            if (iconstr == "02d") icon = 2;
            if (iconstr == "02n") icon = 3;
            if (iconstr == "03d") icon = 4;
            if (iconstr == "03n") icon = 4;
            if (iconstr == "04d") icon = 5;
            if (iconstr == "04n") icon = 5;
            if (iconstr == "09d") icon = 6;
            if (iconstr == "09n") icon = 6;
            if (iconstr == "10d") icon = 7;
            if (iconstr == "10n") icon = 8;
            if (iconstr == "11d") icon = 9;
            if (iconstr == "11n") icon = 9;
            if (iconstr == "13d") icon = 10;
            if (iconstr == "13n") icon = 10;
            if (iconstr == "50d") icon = 11;
            if (iconstr == "50n") icon = 11;
            console.log('Weater icon are ' + iconstr);
            
            // Assemble dictionary using our keys
            var dictionary = {
                'KEY_TEMPERATURE': temperature,
                'KEY_ICON': icon
            };
            
            // Send to Pebble
            Pebble.sendAppMessage(dictionary,
                function(e) {
                    console.log('Weather info sent to Pebble successfully!');
                },
                function(e) {
                    console.log('Error sending weather info to Pebble!');
                }
            );
        }      
    );
}

function locationSuccess(pos) {
    // Construct URL
    var url = 'http://api.openweathermap.org/data/2.5/weather?lat='+pos.coords.latitude+'&lon='+pos.coords.longitude;  
    fireWeather(url);

}

function locationError(err) {
    console.log('Error requesting location!');
}
function getWeather() {
    var gps = true;
    if (localStorage.use_gps)
        gps = JSON.parse(localStorage.use_gps);
    
    if (gps)
    {
        navigator.geolocation.getCurrentPosition(
          locationSuccess,
          locationError,
          {timeout: 15000, maximumAge: 60000}
        );   
    }
    else
    {
        var url = "http://api.openweathermap.org/data/2.5/weather?q="+localStorage.city;
        fireWeather(url);
    }

}

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);

Pebble.addEventListener('showConfiguration', function() {
    var url = 'http://tsdo.in/pbt_m/?ver='+CURRENT_VERSION;
    console.log('Showing configuration page: ' + url);

    Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
    var configData = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration page returned: ' + JSON.stringify(configData));
    
    localStorage.temp_unit = configData.temp_unit;
    
    localStorage.use_gps = configData.use_gps;
    localStorage.city = configData.city;

    var dict = {
        "KEY_ADTZ" : JSON.parse(configData.time_zone),
        "KEY_BLUETOOTH_VIBE" : JSON.parse(configData.bluetooth),
        "KEY_HOURLY_VIBE" : JSON.parse(configData.hourly),
        'KEY_HOUR_RED' : parseInt(configData.hour_color.substring(2, 4), 16),
        'KEY_HOUR_GREEN' : parseInt(configData.hour_color.substring(4, 6), 16),
        'KEY_HOUR_BLUE' : parseInt(configData.hour_color.substring(6), 16),
        'KEY_MINUTE_RED' : parseInt(configData.minute_color.substring(2, 4), 16),
        'KEY_MINUTE_GREEN' : parseInt(configData.minute_color.substring(4, 6), 16),
        'KEY_MINUTE_BLUE' : parseInt(configData.minute_color.substring(6), 16),
        'KEY_SECOND_BAR_RED' : parseInt(configData.second_color.substring(2, 4), 16),
        'KEY_SECOND_BAR_GREEN' : parseInt(configData.second_color.substring(4, 6), 16),
        'KEY_SECOND_BAR_BLUE' : parseInt(configData.second_color.substring(6), 16),
        'KEY_POWER_SAVING_START' : JSON.parse(configData.start_time),
        'KEY_POWER_SAVING_END' : JSON.parse(configData.end_time),
        'KEY_SECOND_BAR' : JSON.parse(configData.second_bar),
        'KEY_SECOND_RED' : parseInt(configData.second_num_color.substring(2, 4), 16),
        'KEY_SECOND_GREEN' : parseInt(configData.second_num_color.substring(4, 6), 16),
        'KEY_SECOND_BLUE' : parseInt(configData.second_num_color.substring(6), 16),
        'KEY_SECOND_REFRESH_RATE' : JSON.parse(configData.show_second),
        'KEY_12HOUR' : JSON.parse(configData.c12hour) ? 1 : 0,
    }; 
  
    // Send to watchapp
    Pebble.sendAppMessage(dict, function(e) {
        console.log('Send successful: ' + JSON.stringify(dict));
    }, function(e) {
        console.log('Send failed!');
    });
    
    getWeather();
    
});