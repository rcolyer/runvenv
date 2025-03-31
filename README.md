# runvenv

The primary purpose of runvenv is to solve the problem of launching executable python scripts within a named environment using the shebang system of Unix-like systems.  For example:

`testscript.py:`
```
#!/usr/bin/env -S runvenv myscriptenv

import sys
print(sys.executable)
```

With the above script marked as an executable file, a Unix-like system will execute "`/usr/bin/env -S runvenv myscriptenv testscript.py`".  Then runvenv will look in its search path for the environment named `myscriptenv`, find the python executable within that environment, and execute `testscript.py` with it.  Preferencing the usage of `/usr/bin/env` to call runvenv allows the same shebang line to work identically across platforms, and both with and without root access for installing runvenv, requiring only a common environment name.  For single system tools, the direct path location of runvenv can be placed on the shebang line.

## Search path

When invoked, runvenv will look for venv or conda python environments with the given name in the following order, silently skipping over options which do not exist:

1. In the realpath directory in which the script is located.
2. In `~/.venv/`
3. In `/usr/local/share/venv/`
4. Environments found in `~/.conda/environments.txt`
5. Environments found in `envs_dirs:` directory entries in the `~/.condarc` yaml file.
6. In `~/.conda/envs/`

The first match found of a `myscriptenv/bin/python` will be used to execute the script.

## Usage examples

### Positionally invariant local environment support

Create a git repo with a `script.py` that requires an environment, and provide support for creating a hidden `./.venv/` directory, listed in `.gitignore`, which serves as its enviroment.  Create a symlink in `~/bin/script.py` to the `script.py` within the repo, and set `script.py` to have the shebang line:

```
#!/usr/bin/env -S runvenv .venv
```

Then the symlink within the path in `~/bin/script.py` will be executed within the environment setup in the repo directory, executing `repo/.venv/bin/python`.

### Shared multi-use user environments

Add an environment to `~/.venv` with "`python -m venv ~/.venv/mysharedenv`" and then set scripts to use this with:

```
#!/usr/bin/env -S runvenv mysharedenv
```

### System-level sharing of environments

This example is for system-level scripts requiring environments, or for multi-user systems (e.g. for data analysis) benefitting from users having access to a common environment. Create environments under `/usr/local/share/venv/` such as `/usr/local/share/venv/mysystemenv` which are readable by all, and set scripts to use:

```
#!/usr/bin/env -S runvenv mysystemenv
```

### conda environments

Create a conda environment, enabling support for different python versions, such as with "`conda create -n py313 python=3.13`" and then use this without activating any environment by setting scripts to use:

```
#!/usr/bin/env -S runvenv py313
```

### Versions of pip run directly within other environments

`pip313:`
```
#!/usr/bin/env -S runvenv py313
import re
import sys
from pip._internal.cli.main import main
if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(main())
```

While the example environment here started as a conda environment, and one can certainly use conda to manage this, it is also possible if it fits one's purpose to work with pip on top of a conda start such as: "`pip313 install ipython`".

### Running ipython directly within other environments

`ipython313:`
```
#!/usr/bin/env -S runvenv py313
import re
import sys
from IPython import start_ipython
if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(start_ipython())
```

## Building

Run `make`.  There are no non-standard dependencies.

## Installing

Copy the produced runvenv executable into `/usr/bin/` or `/usr/local/bin/` for a system level install, or somewhere in the path like `~/bin/` for a user level install.

# License

The main part of runvenv is licensed under GPLv3, with the included RCLib files being separately licensed under the permissive Boost license.

