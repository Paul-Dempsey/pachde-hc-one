#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;
	p->addModel(modelHc1);
	p->addModel(modelHc2);
	p->addModel(modelFavorites);
	//p->addModel(modelHc4);
	p->addModel(modelPedal1);
	p->addModel(modelPedal2);
	p->addModel(modelRound);
	p->addModel(modelCompress);
	p->addModel(modelTilt);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
