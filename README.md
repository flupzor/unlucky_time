Unlucky time
=========

A dynamically loaded library which overrides the gettimeofday function in
order to make weird date shifts. Which is useful to test certain software
which shouldn't be dependent on time.

Usage
-----

```
make test
```

If this doesn't return: "All tests ran successfully." the code should be
modified in order to make it work on your platform. Send me an e-mail if this
is the case.

Then on either platform:

```
make
./run.sh ./example.py
```

Note that on OpenBSD the binaries in /bin and /sbin/ are statically compiled
and won't run the dynamic linker. Which means that for those binaries it isn't
possible to make date shifts using the unlucky_time tool.

TODO
----

* There is no thread safety.

* It might be a good idea to print a big fat warning somewhere that you
  shouldn't use this code in production systems.

* I haven't looked into fork(2)ing yet. Might be possible to support
  application which fork(2) as well.

* Use GNU autoconf.
