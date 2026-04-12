# App

UI interface to create BF files or projects, edit, compile, emulate and debug them.

## Structure of the project

The program execution is governed by a state machine.
Each state is rendered using a MVVM approach.
Communication between UI elements happens via synchronous events.

There are several UI tools in the state dedicated to editing the code.
Most of them are self explanatory (compiler output, memory view...).<br>
The `EmulationImageTool` renders the first 16x16 bytes of tape memory,
one per pixel, either decoding them as RGB332 or black/white.
Try decompressing badapple.7z, opening it in the App and emulating it.