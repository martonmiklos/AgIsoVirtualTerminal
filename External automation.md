### External automation

The AgIsoVT can be controlled by external tools through a TCP/IP connection after enabled by the --enable-automation command line argument.

By default it listens on the 4444 TCP port. Other ports can be selected by the --automation-port command line option.

### Automation commands

The following commands are interpreted by the AgIsoVT:

#### Working set managment commands

##### list-working-sets/lws

Lists the working set clients connected to the AgIsoVT.

##### get-active-working-set/gaws

Prints information on the currently active working set

##### select-active-working-set/saws <J1939 NAME> or working set index (1-n)

Sets the currently active working set

#### Softkey interaction

##### press-softkey/psk <softkey index> [press time in ms]

Emulates the press and release of a softkey at the given index

##### hold-softkey/hsk <softkey index>

Emulates the press and holding of a softkey at the given index. Can be released with the release-softkey command.

Softkeymask changes which removing the softkey under the index from the mask will also release the holding of the softkey.

##### release-softkey/rsk <softkey index>

Emulates the releasing of a softkey at the given index.

#### User input manipulation

##### press-button/pbtn <button object ID> [press time in ms]

Emulates the pressing and releaseing of a button.
Only works for buttons visible on the active data mask.

##### hold-button/hbtn <button object ID>

Emulates the press and holding of a button. Can be released with the following release-softkey command.

Datamask/alarm mask changes or button hide will cancel the pressed state.

##### release-button/rbtn <button object ID>

Emulates the releasing of a button, does nothing if the given button was not pressed.

#### Screenshot managment

##### take-screenshot/tsh <image path> [datamask/softkeymask]

Captures the current screen (or part of the screen if specified by the optional arguments) and writes the content to a file.

#### IOP managment

##### export-ws-iop <iop path> [working set index]

Exports the IOP contents to a file

