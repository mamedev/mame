.. _contributing:

Contributing to MAME
====================

So you want to contribute to MAME but aren’t sure where to start?  Well
the great news is that there’s always plenty to do for people with a
variety of skill sets.


.. _contributing-testing:

Testing and reporting bugs
--------------------------

One thing MAME can always do with is more testing and bug reports.  If
you’re familiar with a system that MAME emulates and notice something
wrong, or if you find a bug in MAME’s user interface, you can head over
to `MAME Testers <https://mametesters.org/>`_ and, assuming it isn’t
already reported, register an account and open an issue.  Be sure to
read the `FAQ <https://mametesters.org/faq.html>`_ and
`rules <https://mametesters.org/rules.html>`_ first to ensure you start
out on the right foot.  Please note that MAME Testers only accepts
user-facing bugs in tagged release versions.

For other kinds of issues, we have
`GitHub Issues <https://github.com/mamedev/mame/issues/>`_.  There’s a
bit more leeway here.  For example we accept developer-facing issues
(e.g. bugs in internal APIs, or build system inadequacies), feature
requests, and major regressions before they make it into a released
version.  Please respect the fact that the issue tracker is *not* a
discussion or support forum, it’s only for reporting reproducible
issues.  Don’t open issues to ask questions or request support.  Also,
keep in mind that the ``master`` branch is unstable.  If the current
revision doesn’t compile at all or is completely broken, we’re probably
already aware of it and don’t need issues opened for that.  Wait a while
and see if there’s an update.  You might want to comment on the commit
in question with the compiler error message, particularly if you’re
compiling in an unorthodox but supported configuration.

When opening an issue, remember to provide as much information as
possible to help others understand, reproduce, and diagnose the issue.
Things that are helpful to include:

* The incorrect behaviour, and expected or correct behaviour.  Be
  specific: just saying it “doesn’t work” usually isn’t enough detail.
* Environment details, including your operating system, CPU
  architecture, system locale, and display language, if applicable.  For
  video output bugs, note your video hardware (GPU), driver version, and
  the MAME video output module you’re using.  For input handling bugs,
  include the input peripherals and MAME input modules you’re using.
* The exact version of MAME you’re using, including a git commit digest
  if it isn’t a tagged release version, and any non-standard build
  options.
* The exact system and software being emulated (may not be applicable
  for issues with parts of MAME’s UI, like the system selection menu).
  Include things like the selected BIOS version, and emulated peripheral
  (slot device) configuration.
* Steps to reproduce the issue.  Assume the person reading is familiar
  with MAME itself, but not necessarily familiar with the emulated
  system and software in question.  For emulation issues, input
  recordings and/or saved state files for reproducing the issue can be
  invaluable.
* An original reference for the correct behaviour.  If you have access
  to the original hardware for the emulated system, it helps to make a
  recording of the correct behaviour for comparison.


.. _contributing-code:

Contributing to MAME’s source code
----------------------------------

MAME itself is written in C++, but that isn’t the sum total of the
source code.  The source code also includes:

* The documentation hosted on this site (and also included in releases
  as a PDF), written in
  `reStructuredText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_
  markup.
* The supplied :ref:`plugins <plugins>`, written in
  `Lua 5.3 <https://www.lua.org/manual/5.3/>`_.
* Internal layouts for emulated machines that need to display more than
  a simple video screen.  These are an XML application
  :ref:`described here <layfile>`.
* The software lists, describing known software media for systems that
  MAME emulates.  MAME software lists are an XML application.
* The user interface translations, in GNU gettext PO format.  They can
  be edited with a good text editor, or a dedicated tool like
  `Poedit <https://poedit.net/>`_.

Our primary source code repository is
`hosted on GitHub <https://github.com/mamedev/mame/>`_.  We prefer to
receive source code contributions in the form of
`pull requests <https://github.com/mamedev/mame/pulls>`_.  You’ll need
to learn the basics of git distributed version control and familiarise
yourself with the git tools.  The basic process for creating a pull
request is as follows:

* Sign up for an account on GitHub.
* Create a fork of the mamedev/mame repository.
* Create a new branch off the ``master`` branch in your forked
  repository.
* Clone your forked repository, and check out your new branch.
* Make your changes, and build and test them locally.
* Commit your changes, and push your branch to GitHub.
* Optionally enable GitHub Actions for your forked repository, allowing
  your changes to be built on Windows, macOS and Linux.
* Open a pull request to merge your changes into the ``master`` branch
  in the mamedev/mame repository.

Please keep the following in mind (note that not all points are relevant
to all kinds of changes):

* Make your commit messages descriptive.  Please include what the change
  affects, and what it’s supposed to achieve.  A person reading the
  commit log shouldn’t need to resort to examining the diff to get a
  basic idea of what a commit is supposed to do.  The default commit
  messages provided by GitHub are completely useless, as they don’t give
  any indication of what a change is supposed to do.
* Test your changes.  Ensure that a full build of MAME completes, and
  that the code you changed works.  It’s a good idea to build with
  ``DEBUG=1`` to check that assertions compile and don’t trigger.
* Use an enlightening pull request title and description.  The title
  should give a one-line summary of what the overall change affects and
  what it’s supposed to do.  The description should contain more detail.
  Don’t leave the description empty and describe the change in comments,
  as this makes searching and filtering more difficult.
* Be aware that GitHub Actions has opaque resource limits.  It isn’t
  clear when you’re close to the limits, and we’ve had contributors
  banned from GitHub Actions for violating the limits.  Even if you
  appeal the ban, they still won’t tell you what the actual limits are,
  justifying this by saying that if you know the limits, you can take
  steps to evade them.  If you enable GitHub Actions, consider not
  pushing individual commits if you don’t need them to be automatically
  built, or cancelling workflow runs when you don’t need the results.
* If your submission is a computer or other device such as a synthesizer
  or sampler which requires a disk, tape, cartridge, or other media to
  start up and run, please consider creating a software list containing
  at least one example of that media.  This helps everyone making changes
  to shared MAME components to easily verify if the changes negatively
  impact your code.
* When submitting any new non-arcade machine, but especially a machine
  which does not auto-boot and requires some interaction to start up
  and be usable, consider adding usage instructions to the
  `System-Specific Setup and Information <https://wiki.mamedev.org/index.php/System-Specific_Setup_and_Information>`_
  page of the `MAME Wiki <https://wiki.mamedev.org>`_.  Anyone can edit
  the wiki after creating an account, and sub-pages for your system
  which discuss technical details of the system are also welcome.

We have guidelines for specific parts of the source:

.. toctree::
	:titlesonly:

	cxx
	softlist
