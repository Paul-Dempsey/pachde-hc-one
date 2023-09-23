#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	// Add modules here
	p->addModel(modelHc1);
	p->addModel(modelHc2);
	p->addModel(modelHc3);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
