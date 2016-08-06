# Generate CPP for JS based on IDL

See [WebIDL spec](http://heycam.github.io/webidl/) for details

# Usage

Have a look at the Makefile. It is a bit crude, but you'll probably get the idea.

# FAQ

**Q:** _Why are you using a NodeJS module?_

**A:** Allthough we like dogfood, we do not suffer from the NIH syndrome.

**Q:** _Why bother using WebIDL?_

**A:** We like to do other things than constantly tweaking our code with every spidermonkey update.

**Q:** _Why are you focussing on spidermonkey?_


**A:** We started with Spidermonkey around 2007, and kindof stuck with us.
       Sooner or later we will add support for multiple Javascript engines. (V7, V8, chakra, jscore, jerryscript, ..) so that we can take full advantage of the native environments.

