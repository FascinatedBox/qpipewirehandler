qpipewirehandler
================

This repository contains `QPipewireHandler`, a class that listens for items
being added to or removed from the PipeWire graph. This repository also contains
a demo application that logs those events to a QTextEdit.

The repository example is built using Qt 5 and builds with CMake.

### Usage

Add `QPipewireHandler.{cpp,h}` to your project, as well as the CMake rules to
search for PipeWire. The handler **does not** block the GUI thread, so it does
not need to be boxed into a QThread.

When an item is added to the graph, the handler creates a `new` struct
containing the item's properties and fires a Qt signal. The handler **does not**
use the struct after firing the signal. **The caller must free the struct**.

Each `PWItem` contains a `QHash<QString, QString> props` containing the same
info as the `props` section of an object if viewed by `pw-dump`.

### Caveats

The caller is responsible for maintaining a graph based on the items provided by
the handler.

The handler does not provide a means for sending events back to PipeWire, or for
listening to other kinds of events.`

### Why

I'm planning to use this to build tools to do PipeWire automation.
