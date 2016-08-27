ip\_df\_cleaner
=----==========

Description
-----------

NFQUEUE userspace helper to clean IP DF bit.

Compiling
---------

### Prerequisites

* Linux kernel
* cmake (tested with 2.8.11, 3.6.1)
* make (tested with GNU Make 3.82, 4.2.1)
* gcc (tested with 4.8.5, 6.1.1), clang (tested with 3.8.1) or icc (tested with 16.0.3)
* libnetfilter-queue (tested with 1.0.2)

### Compiling

Create `build` folder, chdir to it, then run

`cmake ..`

or

`cmake -DCMAKE_BUILD_TYPE=Debug ..`

to build app with debug info. Then just type `make`.

Usage
-----

The following arguments are supported:

* --queue=&lt;N&gt; (optional) specifies queue to bind to (default: 0).

Typical usage:

`ip_df_cleaner --queue=0`

Also, systemd template unit is available:

```
systemctl enable ip_df_cleaner@0
systemctl start ip_df_cleaner@0
```

Distribution and Contribution
-----------------------------

Distributed under terms and conditions of GNU GPL v3 (only).

The following people are involved in development:

* Oleksandr Natalenko &lt;oleksandr@natalenko.name&gt;

Mail them any suggestions, bugreports and comments.

