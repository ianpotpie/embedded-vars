# Embedded Vars

This project is inspired by the BMExplorer tool used by my coworker/mentor Jim Coughlin.
It is meant to be modified and retrofitted into embedded projects and used to monitor and set values at runtime.

It works by maintaining a global "registry" of references to variables and arrays.
Internally, the registry is an array of var_t stucts, which each maintain the metadata for a memory location.
Each entry in the registry has an associated "path" value.
The path of a variable is a unique identifier and is used in a filesystem-like organization structure.

## Usage

### Server

### Client
