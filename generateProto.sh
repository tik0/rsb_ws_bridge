#!/bin/bash
protoc -Iwww www/*.proto --cpp_out=.
