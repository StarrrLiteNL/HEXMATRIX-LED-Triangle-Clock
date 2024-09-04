#ifndef PAGE_BRIGHTNESS_H
#define PAGE_BRIGHTNESS_H

const char PAGE_Brightness[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="/"  class="btn btn--s btn--grey">&#9664;</a>&nbsp;&nbsp;<strong>Brightness setting</strong>
<hr>
<form action="" method="get">
<table border="0"  cellspacing="0" cellpadding="3" >
<tr><td align="right">Brightness night:</td><td><input type="text" id="brightness_low" name="brightness_low" size="3" maxlength="3" value=""></td></tr>
<tr><td align="right">Sunrise offset (min):</td><td><input type="text" id="offset_sunrise" name="offset_sunrise" size="3" maxlength="3" value=""></td></tr>
<tr><td align="right">Sunrise spread (min):</td><td><input type="text" id="spread_sunrise" name="spread_sunrise" size="3" maxlength="3" value=""></td></tr>
<tr><td align="right">Brightness day:</td><td><input type="text" id="brightness_high" name="brightness_high" size="3" maxlength="3" value=""></td></tr>
<tr><td align="right">Sunset offset (min):</td><td><input type="text" id="offset_sunset" name="offset_sunset" size="3" maxlength="3" value=""></td></tr>
<tr><td align="right">Sunset spread (min):</td><td><input type="text" id="spread_sunset" name="spread_sunset" size="3" maxlength="3" value=""></td></tr>
<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--grey" value="Save"></td></tr>
<tr><td align="right">Sunrise:</td><td><span id="id_sunrise"></span></td></tr>
<tr><td align="right">Sunset:</td><td><span id="id_sunset"></span></td></tr>
<tr><td align="right">Current brightness:</td><td><span id="id_brightness"></span></td></tr>
</table>
</form>
<script>




window.onload = function ()
{
  load("style.css","css", function()
  {
    load("microajax.js","js", function()
    {
        setValues("/admin/brightnessvalues");
    });
  });
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====";

void brightness_Set_html()
{
  if (server.args() > 0 )  // Save Settings
  {
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "brightness_low") config.brightnessLow = server.arg(i).toInt();
      if (server.argName(i) == "brightness_high") config.brightnessHigh = server.arg(i).toInt();
      if (server.argName(i) == "offset_sunrise") config.offsetSunrise = server.arg(i).toInt();
      if (server.argName(i) == "offset_sunset") config.offsetSunset = server.arg(i).toInt();
      if (server.argName(i) == "spread_sunrise") config.spreadSunrise = server.arg(i).toInt();
      if (server.argName(i) == "spread_sunset") config.spreadSunset = server.arg(i).toInt();
    }
    WriteConfig();
  }
  server.send_P ( 200, "text/html", PAGE_Brightness );
  Serial.println(__FUNCTION__);

}

void brightness_Set_values_html()
{
  String values ="";
  values += "brightness_low|" + (String) config.brightnessLow + "|input\n";
  values += "brightness_high|" + (String) config.brightnessHigh + "|input\n";
  values += "offset_sunrise|" + (String) config.offsetSunrise + "|input\n";
  values += "offset_sunset|" + (String) config.offsetSunset + "|input\n";
  values += "spread_sunrise|" + (String) config.spreadSunrise + "|input\n";
  values += "spread_sunset|" + (String) config.spreadSunset + "|input\n";
  values += "id_sunrise|" + (String) (sunRise / 60) + ":" + (sunRise % 60)  + "|div\n";
  values += "id_sunset|" + (String) (sunSet / 60) + ":" + (sunSet % 60)  + "|div\n";
  values += "id_brightness|" + (String) BRIGHTNESS + "|div\n";

  server.send ( 200, "text/plain", values);
  Serial.println(__FUNCTION__);
}

void updateBrightness() {
  int minutesSinceMidnight = (DateTime.hour * 60) + DateTime.minute;
  BRIGHTNESS = config.brightnessHigh;

  if (config.spreadSunrise == 0 ) {
    config.spreadSunrise = 1;

  }
  if (config.spreadSunset == 0 ) {
    config.spreadSunset = 1;
  }

  // Before sunrise ended
  if (minutesSinceMidnight < (sunRise + config.offsetSunrise) + config.spreadSunrise) {
    BRIGHTNESS = config.brightnessLow + ( (float) abs( config.brightnessHigh - config.brightnessLow ) / (float) -(config.spreadSunrise * 2) ) * ( minutesSinceMidnight - ( ( sunRise + config.offsetSunrise ) - config.spreadSunrise ) );
    BRIGHTNESS = max( min( (int) BRIGHTNESS, config.brightnessHigh ), config.brightnessLow );
  }

  // After sunset started
  if (minutesSinceMidnight > ( sunSet + config.offsetSunset ) - config.spreadSunset) {
    BRIGHTNESS = config.brightnessHigh - ( (float) abs( config.brightnessHigh - config.brightnessLow ) / (float) (config.spreadSunset * 2) ) * ( minutesSinceMidnight - ( ( sunSet + config.offsetSunset ) - config.spreadSunset ) );
    BRIGHTNESS = max( min( (int) BRIGHTNESS, config.brightnessHigh ), config.brightnessLow );
  }

  // Serial.print("Calculated brightness : "); Serial.println(BRIGHTNESS);
  FastLED.setBrightness(BRIGHTNESS);
}

void calculateSun() {
  sun.setPosition(LATITUDE, LONGITUDE, TIMEZONE);
  sun.setCurrentDate(DateTime.year, DateTime.month, DateTime.day);
  sun.setTZOffset(summerTime(ConvertDate(DateTime.year, DateTime.month, DateTime.day, DateTime.hour, DateTime.minute, DateTime.second)) ? 2 : 1);
  sunRise = sun.calcSunrise();
  sunSet = sun.calcSunset();
  // Serial.print("Calulating sunset and sunrise for : "); Serial.print(DateTime.day); Serial.print("-"); Serial.print(DateTime.month); Serial.print("-"); Serial.println(DateTime.year);
  // Serial.print("Calulated sunrise: "); Serial.println(sunRise);
  // Serial.print("Calculated Sunset : "); Serial.println(sunSet);
}

#endif
