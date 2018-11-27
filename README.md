# statusbar-networkmanager
## A Blocklet for i3blocks, that displays signalstrength in percentage or decibel.

The code in this repositroy contains code for a daemon, that should be run as root, but also code to send Client-signals via IPC.

The bash-script is called by i3blocks and handles optional mouse input.
⋅⋅⋅On left mouseclick a connection to the selected network will be established⋅⋅⋅
⋅⋅⋅On middle mousebutton the establishes connection will be closed⋅⋅⋅
⋅⋅⋅On right mouseclick the displayed unit will be switched⋅⋅⋅
⋅⋅⋅Scrolling the mouse-wheel will select the next or previous network⋅⋅⋅
