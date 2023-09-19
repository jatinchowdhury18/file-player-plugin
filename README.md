# File Player Plugin

The purpose of this plugin is to experiment with a CLAP-first
plugin development workflow. The current workflow is as follows:
- A CLAP plugin base (using clap-helpers)
- A JUCE plugin GUI (may change in the future)
- CLAP wrappers for VST3, AU, and Standalone

TODOs:
- Fork clap-helpers? (currently need to make some private members protected)
- What about AUv3/iOS?
- clap-wrappers needs some help to work with C++20 (std::string vs. std::u8string)
- Actually implement the engine and UI
- Engine <-> UI communication

More to come!
