#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"
#include <ESP32Servo.h>
// Replace with your network credentials
const char* ssid ="AMAX" ;// "AMAX""TP-Link_809E";//
const char* password = "amin1993" ;//;"""26056937";//

#define PART_BOUNDARY "123456789000000000000987654321"

  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22


#define MOTOR_1_PIN_1    14
#define MOTOR_1_PIN_2    2
#define MOTOR_2_PIN_1    13
#define MOTOR_2_PIN_2    15
//  #define led              4
#define SERVO_1          12
// #define SERVO_2          12
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step

// 1=full step, 2=half step etc.
// All the wires needed for full functionality
const int MOTOR_STEP_PIN = 1; //1
const int MOTOR_DIRECTION_PIN = 3;
const int analogPin = 4;

#define motor_1_1        10
#define motor_1_2        11
#define motor_2_1        12
#define motor_2_2        13
#define steper        2

int sensorValue = 10;                   // Store the sensor reading


// Servo servoT;
Servo servoP;
// BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
// ESP_FlexyStepper stepper;
void initMotors() 
{
  // servoT.setPeriodHertz(50);    // standard 50 hz servo
  servoP.setPeriodHertz(50);    // standard 50 hz servo
    ledcSetup(motor_1_1, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(motor_1_2, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(motor_2_1, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(motor_2_2, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(steper, 1000, 8); // 2000 hz PWM, 8-bit resolution
  ledcAttachPin(MOTOR_1_PIN_1, motor_1_1);
  ledcAttachPin(MOTOR_1_PIN_2, motor_1_2);
  ledcAttachPin(MOTOR_2_PIN_1, motor_2_1);
  ledcAttachPin(MOTOR_2_PIN_2, motor_2_2);
  ledcAttachPin(MOTOR_STEP_PIN, steper);
// stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
//  stepper.setSpeedInStepsPerSecond(200);
//   stepper.setAccelerationInStepsPerSecondPerSecond(100);

  // stepper.begin(RPM, MICROSTEPS);

  servoP.attach(SERVO_1);
  // servoT.attach(SERVO_2);

}


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>Mars Rover</title>
    <meta name="viewport" content="width=auto, initial-scale=1">
    <style>
      body {background-color: #4a5972; font-family: Arial; width: auto; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  
        object-fit: contain ;
        width: 100%;
        max-width: 830px;
        /* width: fit-content; */
        max-height: 600px ; 
        position:absolute; 
        left:258px; 
        top:38px; 
      }
      .row {
        /* --bs-gutter-x: 1.5rem;
        --bs-gutter-y: 0; */
        display: flex;
        flex-wrap: wrap;
        /* margin-top: calc(-1 * var(--bs-gutter-y));
        margin-right: calc(-.5 * var(--bs-gutter-x));
        margin-left: calc(-.5 * var(--bs-gutter-x)); */
        height:600px ;
      }
      .col {
        flex: 1 1 100px;
        
      }


      .slider {
        -webkit-appearance: none; 
        width: 200px;
        height: 15px;
        border-radius: 5px;  
        background: #d3d3d3;
        outline: none;
        opacity: 0.7;
        -webkit-transition: .2s;
        transition: opacity .2s;
      }

      .slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 35px;
        height: 35px;
        border-radius: 50%; 
        background: #10fa1c;
        cursor: pointer;
      }

      .slider::-moz-range-thumb {
        width: 25px;
        height: 25px;
        border-radius: 50%;
        background: #601313;
        cursor: pointer;
      }
      #joystick
      {
        border: 2px solid #fefefe;
      }
       /* toggle in label designing */
       .toggle {
            position : relative ;
            display : inline-block;
            width : 120px;
            height : 52px;
            background-color: rgba(52, 53, 58, 0.982);
            border-radius: 30px;
            border: 2px solid rgb(255, 255, 255);
        }
               
        /* After slide changes */
        .toggle:after {
            content: '';
            position: absolute;
            width: 50px;
            height: 50px;
            border-radius: 50%;
            background-color: rgb(9, 99, 177);
            top: 1px;
            left: 1px;
            transition:  all 0.5s;
        }
               
        /* Toggle text */
        p {
            font-family: Arial, Helvetica, sans-serif;
            font-weight: bold;
        }
               
        /* Checkbox checked effect */
        .checkbox:checked + .toggle::after {
            left : 70px;
        }
               
        /* Checkbox checked toggle label bg color */
        .checkbox:checked + .toggle {
            background-color: green;
        }
               
        /* Checkbox vanished */
        .checkbox {
            display : none;
        }
    </style>
  </head>
  <body>
      <div class="row">
        <div class="col"  style = "max-width: 250px ;height:600px ;  background-color: #34363d; border: 2px dashed #15da71;">
          <h1 style = "font-size: 15px; background-color: #ffffff; border: 2px dashed #15da71;">Environmental surveying</h1>
          <div class="slidecontainer" >

          <p style = "color: white; position:relative; left:0px; top:10px;">sensor motor: <span id="panV"></span></p>
          <!-- <p style = "position:relative; left:-100px; top:20px;">Soil moisture measurement</p> -->
          <input type="range" min="0" max="100" value="50" class="slider" id="pan" style = "transform: rotate(270deg);position:relative; left:0px; top:120px;">
          
        </div>
          <div class="slidecontainer" >

          <!-- <p style = "position:relative; left:50px; top:50px;">tilt</p> -->
          <input type="range" min="1" max="180" value="90" class="slider" id="tilt" style = "transform: rotate(0deg);position:relative; left:0px; top:270px;">
          <p style = "color: white; position:relative; left:0px; top:200px;">Cam Tilt: <span id="tiltV"></span></p>
          <p style = "color: white; position:relative; left:0px; top:300px;">Soil moisture measurement: <span id="PresValue"></span></p>

          </div>
    
  
        </div>
        <div class="col"  style = "height:600px ;  background-color: #4281b8; border: 2px dashed #15da71;">
          <h1 style = "font-size: 15px; position:relative; left:center ; top:-30px; background-color: #ffffff; border: 2px dashed #8e1237;">Mars Rover</h1>

          <img src="" id="photo" > 

        </div>
        <div class="col"  style = "max-width: 250px ;height:600px ;  background-color: #34363d; border: 2px dashed #15da71;">
          <h1 style = "font-size: 15px; background-color: #ffffff; border: 2px dashed #15da71;">Joy Stick</h1>
            <div id="joy1Div" style="width:200px;height:200px;margin:0px; position:relative; left:20px; top:50px;"></div>
              <!-- Posizione X:<input id="joy1PosizioneX" type="text" /><br />
              Posizione Y:<input id="joy1PosizioneY" type="text" /><br />
              Direzione:<input id="joy1Direzione" type="text" /><br /> -->
            <div style = "color: white; position:relative; left:0px; top:80px;">
              X :<input id="joy1X" type="text" /></br>
              Y :<input id="joy1Y" type="text" />
              
            <div  style = " position:relative; left:0px; top:50px;">
                <h1>LED Light</h1>
                <input type="checkbox" id="switch" class="checkbox" onmousedown="toggleCheckbox();" ontouchstart="toggleCheckbox();" />
                <label for="switch" class="toggle">
                 <!-- <p style = " position:relative; left:0px; top:20px;">ON       OFF</p>  -->
                </label>
            </div> 
            <!-- <div class="slidecontainer" >

                <p style = "color: white; position:relative; left:0px; top:80px;">LED Light</span></p>
                <input type="range" min="0" max="1" value="0" class="slidersw" id="led" style = "position:relative; left:0px; top:95px;">
                </div>            -->
        </div>
        </div>
      </div>
    
  
   
<script type="text/javascript">

  
let StickStatus =
{
    xPosition: 0,
    yPosition: 0,
    x: 0,
    y: 0,
    cardinalDirection: "C"
};


var JoyStick = (function(container, parameters, callback)
{
    parameters = parameters || {};
    var title = (typeof parameters.title === "undefined" ? "joystick" : parameters.title),
        width = (typeof parameters.width === "undefined" ? 0 : parameters.width),
        height = (typeof parameters.height === "undefined" ? 0 : parameters.height),
        internalFillColor = (typeof parameters.internalFillColor === "undefined" ? "#00AA00" : parameters.internalFillColor),
        internalLineWidth = (typeof parameters.internalLineWidth === "undefined" ? 2 : parameters.internalLineWidth),
        internalStrokeColor = (typeof parameters.internalStrokeColor === "undefined" ? "#003300" : parameters.internalStrokeColor),
        externalLineWidth = (typeof parameters.externalLineWidth === "undefined" ? 2 : parameters.externalLineWidth),
        externalStrokeColor = (typeof parameters.externalStrokeColor ===  "undefined" ? "#008000" : parameters.externalStrokeColor),
        autoReturnToCenter = (typeof parameters.autoReturnToCenter === "undefined" ? true : parameters.autoReturnToCenter);

    callback = callback || function(StickStatus) {};

    // Create Canvas element and add it in the Container object
    var objContainer = document.getElementById(container);
    
    // Fixing Unable to preventDefault inside passive event listener due to target being treated as passive in Chrome [Thanks to https://github.com/artisticfox8 for this suggestion]
    objContainer.style.touchAction = "none";

    var canvas = document.createElement("canvas");
    canvas.id = title;
    if(width === 0) { width = objContainer.clientWidth; }
    if(height === 0) { height = objContainer.clientHeight; }
    canvas.width = width;
    canvas.height = height;
    objContainer.appendChild(canvas);
    var context=canvas.getContext("2d");

    var pressed = 0; // Bool - 1=Yes - 0=No
    var circumference = 2 * Math.PI;
    var internalRadius = (canvas.width-((canvas.width/2)+10))/2;
    var maxMoveStick = internalRadius + 5;
    var externalRadius = internalRadius + 30;
    var centerX = canvas.width / 2;
    var centerY = canvas.height / 2;
    var directionHorizontalLimitPos = canvas.width / 10;
    var directionHorizontalLimitNeg = directionHorizontalLimitPos * -1;
    var directionVerticalLimitPos = canvas.height / 10;
    var directionVerticalLimitNeg = directionVerticalLimitPos * -1;
    // Used to save current position of stick
    var movedX=centerX;
    var movedY=centerY;

    // Check if the device support the touch or not
    if("ontouchstart" in document.documentElement)
    {
        canvas.addEventListener("touchstart", onTouchStart, false);
        document.addEventListener("touchmove", onTouchMove, false);
        document.addEventListener("touchend", onTouchEnd, false);
    }
    else
    {
        canvas.addEventListener("mousedown", onMouseDown, false);
        document.addEventListener("mousemove", onMouseMove, false);
        document.addEventListener("mouseup", onMouseUp, false);
    }
    // Draw the object
    drawExternal();
    drawInternal();

    /******************************************************
     * Private methods
     *****************************************************/

    /**
     * @desc Draw the external circle used as reference position
     */
    function drawExternal()
    {
        context.beginPath();
        context.arc(centerX, centerY, externalRadius, 0, circumference, false);
        context.lineWidth = externalLineWidth;
        context.strokeStyle = externalStrokeColor;
        context.stroke();
    }

    /**
     * @desc Draw the internal stick in the current position the user have moved it
     */
    function drawInternal()
    {
        context.beginPath();
        if(movedX<internalRadius) { movedX=maxMoveStick; }
        if((movedX+internalRadius) > canvas.width) { movedX = canvas.width-(maxMoveStick); }
        if(movedY<internalRadius) { movedY=maxMoveStick; }
        if((movedY+internalRadius) > canvas.height) { movedY = canvas.height-(maxMoveStick); }
        context.arc(movedX, movedY, internalRadius, 0, circumference, false);
        // create radial gradient
        var grd = context.createRadialGradient(centerX, centerY, 5, centerX, centerY, 200);
        // Light color
        grd.addColorStop(0, internalFillColor);
        // Dark color
        grd.addColorStop(1, internalStrokeColor);
        context.fillStyle = grd;
        context.fill();
        context.lineWidth = internalLineWidth;
        context.strokeStyle = internalStrokeColor;
        context.stroke();
    }

    /**
     * @desc Events for manage touch
     */
    function onTouchStart(event) 
    {
        pressed = 1;
    }

    function onTouchMove(event)
    {
        if(pressed === 1 && event.targetTouches[0].target === canvas)
        {
            movedX = event.targetTouches[0].pageX;
            movedY = event.targetTouches[0].pageY;
            // Manage offset
            if(canvas.offsetParent.tagName.toUpperCase() === "BODY")
            {
                movedX -= canvas.offsetLeft;
                movedY -= canvas.offsetTop;
            }
            else
            {
                movedX -= canvas.offsetParent.offsetLeft;
                movedY -= canvas.offsetParent.offsetTop;
            }
            // Delete canvas
            context.clearRect(0, 0, canvas.width, canvas.height);
            // Redraw object
            drawExternal();
            drawInternal();

            // Set attribute of callback
            StickStatus.xPosition = movedX;
            StickStatus.yPosition = movedY;
            StickStatus.x = (255*((movedX - centerX)/maxMoveStick)).toFixed();
            StickStatus.y = ((255*((movedY - centerY)/maxMoveStick))*-1).toFixed();
            StickStatus.cardinalDirection = getCardinalDirection();
            callback(StickStatus);
            joysend(StickStatus);
        }
    } 

    function onTouchEnd(event) 
    {
        pressed = 0;
        // If required reset position store variable
        if(autoReturnToCenter)
        {
            movedX = centerX;
            movedY = centerY;
        }
        // Delete canvas
        context.clearRect(0, 0, canvas.width, canvas.height);
        // Redraw object
        drawExternal();
        drawInternal();

        // Set attribute of callback
        StickStatus.xPosition = movedX;
        StickStatus.yPosition = movedY;
        StickStatus.x = (255*((movedX - centerX)/maxMoveStick)).toFixed();
        StickStatus.y = ((255*((movedY - centerY)/maxMoveStick))*-1).toFixed();
        StickStatus.cardinalDirection = getCardinalDirection();
        callback(StickStatus);
        joysend(StickStatus);
    }

    /**
     * @desc Events for manage mouse
     */
    function onMouseDown(event) 
    {
        pressed = 1;
    }

    /* To simplify this code there was a new experimental feature here: https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/offsetX , but it present only in Mouse case not metod presents in Touch case :-( */
    function onMouseMove(event) 
    {
        if(pressed === 1)
        {
            movedX = event.pageX;
            movedY = event.pageY;
            // Manage offset
            if(canvas.offsetParent.tagName.toUpperCase() === "BODY")
            {
                movedX -= canvas.offsetLeft;
                movedY -= canvas.offsetTop;
            }
            else
            {
                movedX -= canvas.offsetParent.offsetLeft;
                movedY -= canvas.offsetParent.offsetTop;
            }
            // Delete canvas
            context.clearRect(0, 0, canvas.width, canvas.height);
            // Redraw object
            drawExternal();
            drawInternal();

            // Set attribute of callback
            StickStatus.xPosition = movedX;
            StickStatus.yPosition = movedY;
            StickStatus.x = (255*((movedX - centerX)/maxMoveStick)).toFixed();
            StickStatus.y = ((255*((movedY - centerY)/maxMoveStick))*-1).toFixed();
            StickStatus.cardinalDirection = getCardinalDirection();
            callback(StickStatus);
            joysend(StickStatus);
        }
    }

    function onMouseUp(event) 
    {
        pressed = 0;
        // If required reset position store variable
        if(autoReturnToCenter)
        {
            movedX = centerX;
            movedY = centerY;
        }
        // Delete canvas
        context.clearRect(0, 0, canvas.width, canvas.height);
        // Redraw object
        drawExternal();
        drawInternal();

        // Set attribute of callback
        StickStatus.xPosition = movedX;
        StickStatus.yPosition = movedY;
        StickStatus.x = (255*((movedX - centerX)/maxMoveStick)).toFixed();
        StickStatus.y = ((255*((movedY - centerY)/maxMoveStick))*-1).toFixed();
        StickStatus.cardinalDirection = getCardinalDirection();
        callback(StickStatus);
        joysend(StickStatus);
    }

    function getCardinalDirection()
    {
        let result = "";
        let orizontal = movedX - centerX;
        let vertical = movedY - centerY;
        
        if(vertical >= directionVerticalLimitNeg && vertical <= directionVerticalLimitPos)
        {
            result = "C";
        }
        if(vertical < directionVerticalLimitNeg)
        {
            result = "N";
        }
        if(vertical > directionVerticalLimitPos)
        {
            result = "S";
        }
        
        if(orizontal < directionHorizontalLimitNeg)
        {
            if(result === "C")
            { 
                result = "W";
            }
            else
            {
                result += "W";
            }
        }
        if(orizontal > directionHorizontalLimitPos)
        {
            if(result === "C")
            { 
                result = "E";
            }
            else
            {
                result += "E";
            }
        }
        
        return result;
    }

    /******************************************************
     * Public methods
     *****************************************************/

    /**
     * @desc The width of canvas
     * @return Number of pixel width 
     */
    this.GetWidth = function () 
    {
        return canvas.width;
    };

    /**
     * @desc The height of canvas
     * @return Number of pixel height
     */
    this.GetHeight = function () 
    {
        return canvas.height;
    };

    /**
     * @desc The X position of the cursor relative to the canvas that contains it and to its dimensions
     * @return Number that indicate relative position
     */
    this.GetPosX = function ()
    {
        return movedX;
    };

    /**
     * @desc The Y position of the cursor relative to the canvas that contains it and to its dimensions
     * @return Number that indicate relative position
     */
    this.GetPosY = function ()
    {
        return movedY;
    };

    /**
     * @desc Normalizzed value of X move of stick
     * @return Integer from -100 to +100
     */
    this.GetX = function ()
    {
        return (255*((movedX - centerX)/maxMoveStick)).toFixed();
    };

    /**
     * @desc Normalizzed value of Y move of stick
     * @return Integer from -100 to +100
     */
    this.GetY = function ()
    {
        return ((255*((movedY - centerY)/maxMoveStick))*-1).toFixed();
    };

    /**
     * @desc Get the direction of the cursor as a string that indicates the cardinal points where this is oriented
     * @return String of cardinal point N, NE, E, SE, S, SW, W, NW and C when it is placed in the center
     */
    this.GetDir = function()
    {
        return getCardinalDirection();
    };
});

// Create JoyStick object into the DIV 'joy1Div'
var Joy1 = new JoyStick('joy1Div');

var joy1IinputPosX = document.getElementById("joy1PosizioneX");
var joy1InputPosY = document.getElementById("joy1PosizioneY");
var joy1Direzione = document.getElementById("joy1Direzione");
var joy1X = document.getElementById("joy1X");
var joy1Y = document.getElementById("joy1Y");

setInterval(function(){ joy1IinputPosX.value=Joy1.GetPosX(); }, 100);
setInterval(function(){ joy1InputPosY.value=Joy1.GetPosY(); }, 100);
setInterval(function(){ joy1Direzione.value=Joy1.GetDir(); }, 100);
setInterval(function(){ joy1X.value=Joy1.GetX(); }, 100);
setInterval(function(){ joy1Y.value=Joy1.GetY(); }, 100);

// Create JoyStick object into the DIV 'joy1Div'
//var Joy1 = new JoyStick('joy1Div', {}, function(stickData) {
//    joy1IinputPosX.value = stickData.xPosition;
//    joy1InputPosY.value = stickData.yPosition;
//    joy1Direzione.value = stickData.cardinalDirection;
//    joy1X.value = stickData.x;
//    joy1Y.value = stickData.y;
//});

</script>
<script>


function joysend(joyD){
     var xhr0 = new XMLHttpRequest();
     xhr0.open("GET", "/action?go=" +"joyy"+'&'+"val="+joyD.y, true);
     xhr0.send();
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/action?go=" +"joyx"+'&'+"val="+joyD.x, true);
     xhr.send();
}


setInterval(function() {
  getData();
}, 2000); 
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      const sensorValues = this.responseText;
       document.getElementById("PresValue").innerHTML = sensorValues;
    }
  };
  xhttp.open("GET", "readSensor", true);
  xhttp.send();
}


  var sliderp = document.getElementById("pan");
   var outputp = document.getElementById("panV");

        // Set the default value
        const defaultValue = 50;

        // Function to reset the slider value to the default value
        function resetSlider() {
            sliderp.value = defaultValue;
            outputp.innerHTML =sliderp.value;
            var xhr1 = new XMLHttpRequest();
            xhr1.open("GET", "/action?go=" +"panV"+'&' +"val="+ sliderp.value, true);
            xhr1.send();
        }
        // Add an event listener to reset the slider value on touch end
        sliderp.addEventListener("onTouchEnd", resetSlider);
        sliderp.addEventListener("mouseup", resetSlider);
outputp.innerHTML = sliderp.value;
   sliderp.oninput = function() {
//   var xhr1 = new XMLHttpRequest();
   outputp.innerHTML =this.value ;
//    xhr1.open("GET", "/action?go=" + "panV", true);
//    xhr1.send();
    var xhr1 = new XMLHttpRequest();
    xhr1.open("GET", "/action?go=" +"panV"+'&' +"val="+ sliderp.value, true);
    xhr1.send();
     
   }
   
   var slidert = document.getElementById("tilt");
   var outputt = document.getElementById("tiltV");
   outputt.innerHTML = slidert.value;

   slidert.oninput = function() {
   var xhr2 = new XMLHttpRequest();
   outputt.innerHTML =this.value ;

    xhr2.open("GET", "/action?go=" + "tiltV" + "&"  +"val="+ slidert.value, true);
    xhr2.send();
     
   }
   var ledsw = 0;
   var mask = 1;
//    var ledsw = document.getElementById("switch");
    function toggleCheckbox() {
    ledsw ^= mask;
    var xhr3 = new XMLHttpRequest();
    xhr3.open("GET", "/action?go=" +"led"+'&' +"val="+ ledsw, true);
    xhr3.send();
     
   }
 
  window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
 </script>
  </body>
</html>
)rawliteral";

////////////////////////////////////////////////////////////////////////////
static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      // Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            // Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

int valueY = 0 , valueX = 0 , valueTilt = 90 , valuePan = 90 ;
bool cc=1;
int ledV=0;


static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  char value[32];
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable))!= ESP_OK || httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
      free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      } else {
        
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    // Serial.print("buf: ");
    // Serial.println(buf);
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  if(cc=1){
     s->set_dcw(s, 1);
   s->set_lenc(s, 1);  
   cc=0;
  }
  s->set_vflip(s, 1);
   int res = 0;
   int val = atoi(value);

  if(!strcmp(variable, "joyx")) {
    valueX = val ;
  } else if(!strcmp(variable, "joyy")) {
    valueY = val ;
  } else if(!strcmp(variable, "tiltV")) {
    valueTilt = val ;
  } else if(!strcmp(variable, "panV")) {
    valuePan = int(val) ;
  }else if(!strcmp(variable, "led")) {
    if(!strcmp(value, "on")){
      ledV=0;
    }else{
      ledV=0;
    }
  }

//  digitalWrite(led,ledV);

  // Serial.print("joyx: ");
  // Serial.print(valueX);
  // Serial.print(" ,joyy: ");
  // Serial.print(valueY);
  // Serial.print("  ,tiltV: ");
  // Serial.print(valueTilt);
  // Serial.print("  ,panV: ");
  // Serial.print(valuePan);
  // Serial.print("  ,led: ");
  // Serial.println(ledV);

//map(valueTilt,1,180,3250 , 6500);
map(valueTilt,1,180,3250 , 6500);
//3250 , 6500
   sensorValue = digitalRead(analogPin);

  servoP.write(valueTilt);
   if(valuePan>50){
    ledcWrite(steper,(valuePan*2));
    digitalWrite(MOTOR_DIRECTION_PIN,1);
  }else if(valuePan<50){
    ledcWrite(steper,((50-valuePan)*2));
    digitalWrite(MOTOR_DIRECTION_PIN,0);
  }else{
    ledcWrite(steper,0);

  }

//   servoT.write(valueTilt);
     if(valueY>=0){
       if(valueX>0){
         //L=y+x R=y
         ledcWrite(motor_1_1,valueY);
         ledcWrite(motor_1_2,0);
         ledcWrite(motor_2_1,(valueX+valueY));
         ledcWrite(motor_2_2,0);
       }else if(valueX<0){
         //L=y R=y-x
         ledcWrite(motor_1_1,(valueY-valueX));
         ledcWrite(motor_1_2,0);
         ledcWrite(motor_2_1,(valueY));
         ledcWrite(motor_2_2,0);
       }
       }else if(valueY<0){
       if(valueX>0){
         //L=y+x R=y
         ledcWrite(motor_1_1,0);
         ledcWrite(motor_1_2,abs(valueY));
         ledcWrite(motor_2_1,0);
         ledcWrite(motor_2_2,(valueX+abs(valueY)));
       }else if(valueX<0){
         //L=y R=y-x
          ledcWrite(motor_1_1,0);
         ledcWrite(motor_1_2,(abs(valueY)-valueX));
         ledcWrite(motor_2_1,0);
         ledcWrite(motor_2_2,(valueY));
       }
    
       }
  
  

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t readSensor_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/plane");

    float pres_result = sensorValue;

    String valuesStrg =  String(pres_result);
    int strgLength = valuesStrg.length();
    char values_as_char[strgLength];
    valuesStrg.toCharArray(values_as_char, strgLength);

    return httpd_resp_send(req,  (const char *)values_as_char, HTTPD_RESP_USE_STRLEN);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
    httpd_uri_t readSensor_uri = {
        .uri       = "/readSensor",
        .method    = HTTP_GET,
        .handler   = readSensor_handler,
        .user_ctx  = NULL
    };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
     httpd_register_uri_handler(camera_httpd, &readSensor_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
   pinMode(MOTOR_1_PIN_1, OUTPUT);
   pinMode(MOTOR_1_PIN_2, OUTPUT);
   pinMode(MOTOR_2_PIN_1, OUTPUT);
   pinMode(MOTOR_2_PIN_2, OUTPUT);
   pinMode(MOTOR_STEP_PIN, OUTPUT);
   pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
   pinMode(analogPin, INPUT);
  // pinMode(led, OUTPUT);
  

  // Serial.begin(115200);
  // Serial.setDebugOutput(false);
  

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; //PIXFORMAT_YUV422
  config.fb_location = CAMERA_FB_IN_PSRAM;
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 20;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
    // Serial.println("psram Found ");
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 40;
    config.fb_count = 1;
    // Serial.println("psram not Found ");
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  
  if (err != ESP_OK) {
    // Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

    initMotors();


  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println("");
  // Serial.println("WiFi connected");
  
  // Serial.print("Camera Stream Ready! Go to: http://");
  // Serial.println(WiFi.localIP());
  // Start streaming web server
  startCameraServer();

        
}

void loop() {
 
}
