Developer Guide
===============

Contributions are more than welcome, and greatly appreciated.

We expect any contributions to be done through Pull-Requests.
We also appreciated comments and issues through the Issues tracker.

General Code Conventions
-------------------------

* No trailing whitespace; no trailing empty lines
* No TABs
* Indent set at four spaces

C++ Specific
------------

* “private:” and “public:” tags aligned with “class” keyword
* Class name start with a capital letter
* Filename all lowercase and using dash as word separator
* Header file use “#pragma once”
* Private members start with an underscore (“_”) prefix
* Keep your lines to about 80 characters or less in length
* No copy-and-paste from an open source projects. Clean code is an important requirement. If inserting code taken from other source, specify licenses together with the pull request, and we will evaluate if there is not conflicts.
* When there is something not covered in these instructions, we follow `this guide <https://google.github.io/styleguide/cppguide.html>`_.

Workflow Conventions
--------------------

We follow `this workflow model <http://nvie.com/posts/a-successful-git-branching-model/>`_.

* Develop/debug on a branch, started at "develop" (e.g., git checkout develop; git checkout –b <NEW-FEATURE>).

* Prefer smaller, independent commits (make them ideally suited for “cherry-picking”)

    * First line of commit message descriptive of the commit; end with a period (‘.’)
    * One empty line, if more detailed explanations are needed
    * More detailed explanations in as many additional lines as needed
    * Use present tense and active voice (“Fix bug #8848” and not “Bug #8848 fixed” or “Fixed bug #8848”)
    * Keep your lines in the commit message to about 80 characters or less in length

* When ready, create a pull request against the "develop" branch.
