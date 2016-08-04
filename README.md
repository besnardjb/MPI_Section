# MPI_Section

This repository implements the MPI_Section interface.

Purpose
-------

The idea of this interface is to explore the usability of  a callback oriented interface in MPI for profiling, it is an initial implementation but already provides a functional instrumentation with the MALP profiling tool (http://malp.hpcframework.com).

Its purpose is to do phase outlining to explore load balancing issues in MPI codes without using tool-specific instrumentation.

Use
---

Add sections inside your code, by pushing and popping contexts, using respectively *MPI_Section_enter* and *MPI_Section_pop* each time you enter a context provide your label.

If no tool is linked/pre-loaded the Push/Pop calls are doing nothing, however if a tool defines the callback it is called (weak symbol).
For state tracking the implementation does maintain an opaque 64bits argument between matching enter and pop callbacks.

