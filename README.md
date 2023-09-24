# File Player Plugin

The purpose of this plugin is to experiment with a CLAP-first
plugin development workflow. The current workflow is as follows:
- A CLAP plugin base (using clap-helpers)
- CLAP wrappers for VST3, AU, and Standalone
- CLAP GUI extension forwards to a JUCE GUI implementation (may change in the future)
- Most other CLAP extensions forward to Jai implementations
- `clap_process` method is implemented in C++ but forwards to Jai (this may change)

TODOs:
- What about AUv3/iOS?

More to come!
