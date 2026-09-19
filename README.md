# clihelp

A Linux command cheatsheet you can use straight from the terminal.
Pick a category, get colorful, copy-ready commands.

## Install

Download the latest `.deb` from [Releases](https://github.com/Amannnrt/clihelp/releases), then:

```bash
sudo apt install ./clihelp_1.0.0_amd64.deb
```

## Usage

```bash
clihelp             # open the menu and pick a category
clihelp --version   # show version
```

Colors turn off automatically when piped, or if `NO_COLOR` is set.

## Build from source

```bash
make          # compile
make run      # run without installing
make deb      # build a .deb into build/
sudo make install
```

## Add or edit cheatsheets

Cheatsheets are plain text files in `data/`:

```
# SECTION TITLE

  Description of the command
  $ the command --here
```

## License

MIT (or whichever you choose)
