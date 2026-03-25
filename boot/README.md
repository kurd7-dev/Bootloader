# boot/

This directory is for embedded blobs linked into the bootloader image.

Planned artifacts:
- `kernel.bin` (raw Image) or Android legacy boot image payload
- optional embedded DTB blob for fallback

Current status:
- no blobs are embedded by default.
