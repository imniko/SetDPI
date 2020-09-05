This is a sample and simple C++ console project to change windows Scale and Layout aka DPI scaling from command line paremeters. It can work on single and multi-monitor setup.

**How to use** 

It takes up to 2 parameters, first being the monitor ID, which you get from windows display settings, if you click identify it shows you the no. 

`SetDPI.exe [monitor no] [DPI scale value]` 

To set 2nd monitor to a scale value of 250, 

`SetDPI.exe 2 250` 

If you have only one monitor or want only main monitor to change you can omit the first parameter, so 

`SetDPI.exe 150`

would set the first or main monitor to 150 scale. 

**Release** 

Get the release build and a sample ahk script with hotkey example from here https://github.com/imniko/SetDPI/releases/

**Credits** 

The whole credits goes to @lihas of this project, https://github.com/lihas/windows-DPI-scaling-sample it was not possible without his findings. 
