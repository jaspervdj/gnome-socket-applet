gnome-socket-applet
===================

Installation
------------

First, put this somewhere in your `.profile` or similar:

    export GNOME_SOCKET_APPLET_PORT=13525

Installing:

    make
    sudo make install

Run:

    gnome-panel --replace &

You should now be able to add it. To update the text of the applet, simply
open a TCP connection using the specified port, write the text and close the
socket again.

XMonad
------

I use the applet with XMonad. I simply have XMonad write it's logs to the socket
using the `logHook`.

    writeLog :: String -> IO ()
    writeLog message = safely $ do
        port <- getEnv "GNOME_SOCKET_APPLET_PORT"
        addrInfo <- head <$> getAddrInfo Nothing (Just "localhost") (Just port)
        sock <- socket (addrFamily addrInfo) Stream defaultProtocol
        connect sock $ addrAddress addrInfo
        handle <- socketToHandle sock WriteMode
        hPutStr handle message
        hClose handle
      where
        safely x = catch x $ const $ return ()

Then, in your `logHook`:

    myLogHook = dynamicLogWithPP $ defaultPP
        { ppOutput = writeLog
        }
