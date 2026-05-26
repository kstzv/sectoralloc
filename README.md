# sectoralloc-allocator

Experimental bitmap-based memory allocator written in pure C.

## Overview

`sectoralloc-allocator` is a low-level allocator designed around:
- sector-based memory organization,
- bitmap-driven allocation,
- deterministic memory traversal,
- fixed-size pages and blocks,
- reduced fragmentation-oriented allocation strategies.

The allocator currently targets:
- embedded systems,
- RTOS-oriented environments,
- low-level systems programming,
- custom runtime experimentation.

## Architecture

Memory layout:
- 64 sectors
- 32 pages per sector
- 4096-byte pages
- 16-byte blocks

Allocation search hierarchy:
1. sector bitmap
2. page bitmap
3. page block bitmap

The allocator supports:
- fragmented allocation search,
- tail allocation search,
- linear sequential allocation,
- atomic sector locking.

## Current Status

Early experimental stage.

Implemented:
- core allocator structures,
- bitmap hierarchy,
- sector/page metadata design,
- allocation model planning.

In progress:
- allocation logic,
- free(),
- synchronization validation,
- fragmentation handling,
- stress testing.

## Goals

- deterministic allocation behavior,
- low-overhead metadata,
- predictable traversal,
- RTOS-friendly architecture,
- portable ISO C implementation.

## License

Dual-licensed under:

- MIT License
- GPL-2.0

SPDX-License-Identifier: GPL-2.0 OR MIT
