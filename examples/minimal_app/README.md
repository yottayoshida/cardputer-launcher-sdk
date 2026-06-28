# Minimal App Example

This example shows the shape of a future app contribution. In v0.1.0 apps are compiled into the firmware and registered in `src/main.cpp`.

```cpp
class MinimalApp : public cardputer_launcher::App {
 public:
  const char* id() const override { return "minimal"; }
  const char* name() const override { return "Minimal"; }
  void onInput(cardputer_launcher::AppContext&, const cardputer_launcher::InputEvent&) override {}
  void render(cardputer_launcher::AppContext& ctx) override {
    ctx.display.clear();
    ctx.display.line(0, "Minimal");
    ctx.display.line(2, "Hello Cardputer");
  }
};
```

