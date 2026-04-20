#!/bin/bash

if ! command -v blender &> /dev/null; then
    echo "[ERREUR] Please install blender to use this command."
    exit 1
fi

make assets

