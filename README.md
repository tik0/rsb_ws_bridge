# rsb_ws_bridge

This is a minimal example of "how to" get binary data from a RSB scope, sending it via WebSockets and deserialize it in JavaScript

```bash
# Check it out
git clone --recursive https://github.com/tik0/rsb_ws_bridge.git
# Bootstrap
cd rsb_ws_bridge && mkdir build && cd build && cp -R ../www/ . && cmake ..
# Build it
make
# Setup e.g. spread
spread &
# Run the webserver
./build
# Open your webbrowser @ localhost:8080 and hit the button <Set Scope>
# "Set scope to: /myTwists" should appear in the console
# Play the bag file
bag-play ../twists.tide
# The deserialized data should appear in the log window of the website
```
