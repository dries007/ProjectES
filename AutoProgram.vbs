set WshShell = WScript.CreateObject("WScript.Shell")
WshShell.run("telnet.exe")
WScript.Sleep 500
WshShell.SendKeys "o 127.0.0.1 4444{Enter}"
WScript.Sleep 500
WshShell.SendKeys "reset halt{Enter}"
WScript.Sleep 500
WshShell.SendKeys "flash write_image erase " + WScript.Arguments(0) + "{Enter}"
WScript.Sleep 500
WshShell.SendKeys "reset halt{Enter}"
WScript.Sleep 500
WshShell.SendKeys "exit{Enter}"
WScript.Sleep 500
WshShell.SendKeys "{Enter}"
WScript.Sleep 500
WshShell.SendKeys "q{Enter}"
WScript.Quit
