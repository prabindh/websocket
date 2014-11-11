
/* index.html.h */

char htmlpage2[] =
"<html>\
<title>Industrial Communications Demonstrator></title>\
<head>\
<script type=\"text/javascript\">\
function xmlget()\
{\
request = new XMLHttpRequest();request.open('GET', './getdata.dat', true);request.send();\
request.onload = function()\
{\
alert(request.responseText);\
}\
}\
function setTick(){\
setInterval(function(){xmlget()},10000);\
}\
</script>\
</head>\
<body onload=\"setTick()\">\
<a href=\"getdata.dat\">GET</a>\
<a href=\"putdata.dat\">PUT</a>\
</body>\
</html>\
";


char htmlpage3[] =
"<html>\
<title>Industrial Communications Demonstrator></title>\
<head>\
<script type=\"text/javascript\">\
var maxValueMultiplier = 99;\
function degreesToRadians(value) {\
    return Math.PI / 180 * value;\
};\
function overwriteOptions(existingOptions, newOptions) {\
    for (var key in newOptions) {\
        if (typeof(newOptions[key]) == 'object') {\
            overwriteOptions(existingOptions[key], newOptions[key]);\
        }\
        else {\
            existingOptions[key] = newOptions[key];\
        }\
    }\
};\
var Upp = {\
	Gauge: function(options) {\
        this.redraw = function() {\
            this.drawGauge();\
            this.drawNeedle();\
        };\
		this.getDefaultOptions = function()\
		{\
            var options = {\
                minValue: 0,\
                maxValue: 1,\
                startValue: 0,\
                unit: \"\",\
                minAngle: 60,\
                maxAngle: 300,\
                majorTickStep: 15,\
                minorTickStep: 5,\
                renderTo : 'realtime-gauge',\
                bounce : {\
                    freq: 3,\
                    amplitude: 0.1,\
                    decay: 2.0\
                }\
            };\
			return options;\
		};\
        this.bounceAtValue = function(diff) {\
            var freq = this.options.bounce.freq;\
            var amplitude = this.options.bounce.amplitude * Math.abs(diff);\
            var decay = this.options.bounce.decay * ((this.options.maxValue - this.options.minValue) / Math.abs(diff));\
            var previousValue = this.currentValue;\
            var self = this;\
            var time = 0;\
            this.timerId = setInterval(function() {\
                if (time < 1) {\
                    time += 0.01;\
                    var bounceValue = Math.sin(freq * time * 2 * Math.PI) / Math.exp(decay * time) * amplitude;\
                    if (diff < 0) self.currentValue = previousValue - bounceValue;\
                    else self.currentValue = previousValue + bounceValue;\
                }\
                else {\
                    clearInterval(self.timerId);\
                    self.timerId = 0;\
                    self.currentValue = previousValue;\
                }\
                self.drawNeedle();\
            }, 10);\
        };\
        this.getOptions = function() {\
            return this.options;\
        };\
        this.setOptions = function(newOptions) {\
            overwriteOptions(this.options, newOptions);\
        };\
        this.getValue = function() {\
            return this.currentValue;\
        };\
        this.drawGauge = function() {\
            var c = document.getElementById(this.options.renderTo);\
            var ctx = c.getContext(\"2d\");\
            var centerX = c.width / 2;\
            var centerY = c.height / 2;\
            var outerRadius = Math.min(centerX, centerY);\
            ctx.save();\
            ctx.beginPath();\
            var grdEdge = ctx.createRadialGradient(centerX, centerY, outerRadius * 0.9, centerX, centerY, outerRadius);\
            grdEdge.addColorStop(1.0, '#AAAAAA');\
            grdEdge.addColorStop(0.0, '#FEFEFE');\
            ctx.fillStyle = grdEdge;\
            ctx.arc(centerX, centerY, outerRadius, 0, degreesToRadians(360), true);\
            ctx.fill();\
            ctx.beginPath();\
            ctx.fillStyle = \"#000000\";\
            ctx.arc(centerX, centerY, outerRadius * 0.9, 0, degreesToRadians(360), true);\
            ctx.fill();\
            ctx.beginPath();\
            var grdScale = ctx.createLinearGradient(centerX - outerRadius, 0, centerX + outerRadius, 0);\
            grdScale.addColorStop(0.1,\"#FF0000\");\
            grdScale.addColorStop(0.42,\"#FFFF00\");\
            grdScale.addColorStop(0.58,\"#FFFF00\");\
            grdScale.addColorStop(0.9,\"#00FF00\");\
            ctx.fillStyle = grdScale;\
            ctx.arc(centerX, centerY, outerRadius * 0.85, 0, degreesToRadians(360), false);\
            ctx.fill();\
            ctx.beginPath();\
            ctx.fillStyle = \"#000000\";\
            ctx.arc(centerX, centerY, outerRadius * 0.75, 0, degreesToRadians(360), true);\
            ctx.fill();\
            ctx.beginPath();\
            ctx.fillStyle = \"#000000\";\
            ctx.moveTo(centerX, centerY);\
            ctx.arc (centerX, centerY, outerRadius * 0.87, degreesToRadians(this.options.minAngle + 90), degreesToRadians(this.options.maxAngle + 90), true);\
            ctx.lineTo(centerX, centerY);\
            ctx.fill();\
            ctx.beginPath();\
            ctx.lineWidth = 1.5;\
            ctx.strokeStyle = \"#FFFFFF\";\
            ctx.arc(centerX, centerY, outerRadius * 0.70, degreesToRadians(this.options.minAngle + 90), degreesToRadians(this.options.maxAngle + 90), false);\
            for (var i = this.options.minAngle; i <= this.options.maxAngle; i++) {\
                if (i % this.options.majorTickStep == 0 ||\
                    i % this.options.minorTickStep == 0) {\
                    var angle = degreesToRadians(i);\
                    var dx1 = Math.sin(angle) * outerRadius * 0.70;\
                    var dy1 = Math.cos(angle) * outerRadius * 0.70;\
                    var dx2 = Math.sin(angle) * outerRadius * 0.65;\
                    var dy2 = Math.cos(angle) * outerRadius * 0.65;\
                    if (i % this.options.majorTickStep == 0) {\
                        dx2 = Math.sin(angle) * outerRadius * 0.60;\
                        dy2 = Math.cos(angle) * outerRadius * 0.60;\
                    }\
                    ctx.moveTo(centerX + dx1, centerY + dy1);\
                    ctx.lineTo(centerX + dx2, centerY + dy2);\
                }\
            }\
            ctx.stroke();\
            ctx.textBaseline = \"top\";\
            ctx.font = \"16px Verdana Bold\";\
            ctx.fillStyle = \"#FFFFFF\";\
            ctx.textAlign = \"right\";\
            var dxMax = Math.sin(degreesToRadians(this.options.minAngle)) * (outerRadius * 0.5);\
            var dyMax = Math.cos(degreesToRadians(this.options.minAngle)) * (outerRadius * 0.5);\
            ctx.fillText(this.options.maxValue*maxValueMultiplier, centerX + dxMax + outerRadius * 0.15, centerY + dyMax + outerRadius * 0.15);\
            ctx.textAlign = \"left\";\
            var dxMin = Math.sin(degreesToRadians(this.options.maxAngle)) * (outerRadius * 0.5);\
            var dyMin = Math.cos(degreesToRadians(this.options.maxAngle)) * (outerRadius * 0.5);\
            ctx.fillText(this.options.minValue, centerX + dxMin - outerRadius * 0.15, centerY + dyMin + outerRadius * 0.15);\
            ctx.restore();\
        };\
        this.drawNeedle = function() {\
            var c = document.getElementById(this.options.renderTo);\
            var ctx = c.getContext(\"2d\");\
            var centerX = c.width / 2;\
            var centerY = c.height / 2;\
            var outerRadius = Math.min(centerX, centerY);\
            var needleWidth = outerRadius * 0.02;\
            var needleLength = outerRadius * 0.5;\
            var textAreaLeft = centerX * 0.6;\
            var textAreaBottom = c.height * 0.8;\
            var textAreaWidth = centerX * 0.8;\
            var textAreaHeight = 18;\
            ctx.save();\
           ctx.beginPath();\
            ctx.fillStyle = \"#000000\";\
            ctx.arc(centerX, centerY, outerRadius * 0.55, degreesToRadians(this.options.minAngle - 15 + 90), degreesToRadians(this.options.maxAngle + 15 + 90), false);\
            ctx.fill();\
            var normalizedValue = (1.0 / (this.options.maxValue - this.options.minValue)) * (this.currentValue - this.options.minValue);\
            var angle = (normalizedValue * (this.options.maxAngle - this.options.minAngle)) + this.options.minAngle;\
            ctx.fillRect(textAreaLeft, textAreaBottom, textAreaWidth, textAreaHeight);\
            ctx.textBaseline = \"top\";\
            ctx.textAlign = \"center\";\
            ctx.font = \"16px Verdana Bold\";\
            ctx.fillStyle = \"#FFFFFF\";\
            ctx.fillText(this.currentValue.toFixed(1)*maxValueMultiplier + this.options.unit, centerX, textAreaBottom, textAreaWidth);\
            this.previousTextValue = this.currentValue;\
            ctx.translate(centerX, centerY);\
            ctx.rotate(degreesToRadians(angle));\
            ctx.beginPath();\
            ctx.fillStyle = \"#FFFFFF\";\
            ctx.moveTo(-needleWidth, 0);\
            ctx.lineTo( needleWidth, 0);\
            ctx.lineTo( 0, needleLength);\
            ctx.fill();\
            ctx.restore();\
        };\
        this.moveToValue = function(newValue) {\
            var previousValue = this.currentValue;\
            var diff = newValue - previousValue;\
            if (Math.abs(diff) == 0) {\
                return;\
            }\
            if (this.timerId > 0) {\
                clearInterval(this.timerId);\
                this.timerId = 0;\
            }\
            var timeStep = 0.01 * ((this.options.maxValue - this.options.minValue) / Math.abs(diff));\
            var self = this;\
            var time = 0;\
            this.timerId = setInterval(function() {\
                if (time < 1) {\
                    time += timeStep;\
                    var moveValue = (1 - Math.cos(0.25 * time * 2 * Math.PI)) * diff;\
                    self.currentValue = previousValue + moveValue;\
                    self.drawNeedle();\
                }\
                else {\
                    clearInterval(self.timerId);\
                    self.timerId = 0;\
                    self.currentValue = newValue;\
                    self.drawNeedle();\
                    self.bounceAtValue(diff);\
                }\
            }, 10);\
        };\
		this.options = this.getDefaultOptions();\
		this.setValue = function(value) {\
			var newValue = value * 1.0;\
			if (this.options.minValue <= newValue && newValue <= this.options.maxValue) {\
				this.moveToValue(newValue);\
			}\
		};\
        this.timerId = 0;\
        this.options = this.getDefaultOptions();\
        this.setOptions(options);\
        this.currentValue = this.options.startValue;\
        this.redraw();\
	}\
};\
function xmlget()\
{\
request = new XMLHttpRequest();request.open('GET', './getdata.dat', true);request.send();\
request.onload = function()\
{\
	gauge.setValue(parseFloat(request.responseText));\
}\
}\
function setTick(){\
	setInterval(function(){xmlget()},3000);\
}\
var gauge;\
function draw_gauge(){\
gauge = new Upp.Gauge();\
gauge.setValue(1);\
setTick();\
};\
</script>\
<body onload=\"draw_gauge()\">\
<canvas id=\"realtime-gauge\"></canvas>\
</body>\
</html>\
";



