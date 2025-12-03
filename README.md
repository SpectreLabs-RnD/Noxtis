# Noxtis — WireGuard Obfuscator

Noxtis is a **lightweight WireGuard obfuscation layer** written in C.  
It provides minimal XOR-based obfuscation for WireGuard UDP packets, making them harder to fingerprint or classify by DPI systems.

---

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [How It Works](#how-it-works)
- [Architecture](#architecture)
- [Installation](#installation)
- [Building](#building)
- [Usage](#usage)
  - [Local Mode](#local-mode)
  - [Remote Mode](#remote-mode)
- [Security Notes](#security-notes)
- [Design Objectives](#design-objectives)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)
- [Maintainers](#maintainers)

---

## Introduction

**Noxtis** sits *before* WireGuard’s UDP socket and *after* the network.  
Its job is simple:

1. Take an outbound WireGuard packet  
2. XOR-obfuscate the entire payload  
3. Forward it to the remote Noxtis instance  
4. De-XOR on the other side and feed it into WireGuard

This results in packets that **do not match WireGuard’s standard byte pattern**, disrupting protocol fingerprints.

---

## Features

- Minimal, auditable C code  
- Two independent components:
  - `noxtis_local`
  - `noxtis_remote`
- Fast XOR loop optimized for high throughput  
- libsodium for secure random nonces  
- Zero-copy packet processing  
- Clean Makefile with separate targets  
- Modular file structure (algorithm, I/O, main)  
- Works on any Linux distribution with raw sockets

---

## How It Works

### 1. Local → Remote
- WireGuard packet leaves client
- Local Noxtis XORs it
- Sends obfuscated packet to server port

### 2. Remote → WireGuard
- Remote Noxtis receives obfuscated packet
- Removes XOR
- Injects raw WireGuard packet into server’s UDP socket

### 3. Reverse direction is identical.

The result: WireGuard is fully functional but **no longer looks like WireGuard** to anyone inspecting traffic.

---

## Architecture

Directory structure:

```
noxtis/
│
├── src/
│   ├── algorithm_local.c
│   ├── algorithm_remote.c
│   ├── io_local.c
│   ├── io_remote.c
│   ├── main_local.c
│   ├── main_remote.c
│   ├── noxtis_local.c
│   ├── noxtis_remote.c
│   └── noxtis.h
├── build/
│   └── (object files)
├── Makefile
└── README.md
```

---

## Installation

### Requirements

- gcc
- make
- libsodium development package
- Linux kernel headers

Example (Debian/Ubuntu):

```
sudo apt install build-essential libsodium-dev
```

---

## Building

### Build local component

```
make local
```

Produces:

```
./noxtis_local
```

### Build remote component

```
make remote
```

Produces:

```
./noxtis_remote
```

### Clean build artifacts

```
make clean
```

---

## Usage

### Local Mode

Example:

```
./noxtis_local
```

### Remote Mode

Example:

```
./noxtis_remote
```

---

## Security Notes

Noxtis does **NOT** replace WireGuard encryption.  
WireGuard remains the cryptographic layer.

Noxtis only adds:

- XOR obfuscation  
- Nonce variability  
- Pre/post-processing layer  
- DPI evasion  
- Packet pattern randomization  

It does **not** aim to be:

- A cryptographic protocol  
- A replacement for WireGuard  
- A secure encryption layer on its own  

It is a **pure obfuscator**.

---

## Design Objectives

- Keep code small and fully auditable  
- Zero dynamic allocations in fast path  
- Support multi-gigabit speeds  
- Avoid complexity creep  
- Linux-first, minimal dependencies  
- Clear separation of:
  - algorithm
  - I/O
  - main application logic

---

## Roadmap

- Optional ChaCha pre-whitening  
- Optional AES-CTR mode  
- Addition of randomized Wireguard/UDP packet lengths 
- MTU auto-detection  
- XDP / eBPF acceleration  
- Multi-threaded batch processing  
- Benchmark suite  

---

## Contributing

1. Fork repository  
2. Create branch  
3. Write changes  
4. Submit merge request  

Coding guideline:

```
K&R style, strict warnings, small functions, minimal heap use
```

---

## License

MIT License.

---

## Maintainers

- SpectreLabs  
