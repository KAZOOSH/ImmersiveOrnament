#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

int main()
{
	ofJson jSettings = ofLoadJson("settings.json")["screens"];

	// creates windows, up to 4 supported

	ofGLFWWindowSettings settings;

	// main window
	settings.setSize(jSettings[0]["size"][0], jSettings[0]["size"][1]);
	settings.setPosition(glm::vec2(jSettings[0]["position"][0], jSettings[0]["position"][1]));
	settings.resizable = false;
	settings.transparent = true;

	auto mainWindow = ofCreateWindow(settings);
	
	auto mainApp = make_shared<ofApp>();

	for (int i=1; i<jSettings.size();++i)
	{
		settings.setSize(jSettings[i]["size"][0], jSettings[i]["size"][1]);
		settings.setPosition(glm::vec2(jSettings[i]["position"][0], jSettings[i]["position"][1]));
		settings.shareContextWith = mainWindow;
		settings.resizable = false;
		auto window = ofCreateWindow(settings);
		window->setVerticalSync(false);

		switch (i)
		{
		case 1: {
			ofAddListener(window->events().draw, mainApp.get(), &ofApp::drawWindow2);
			ofAddListener(window->events().keyPressed, mainApp.get(), &ofApp::keyPressedWindow2);

			break;
		}
		case 2: {
			ofAddListener(window->events().draw, mainApp.get(), &ofApp::drawWindow3);
			break;
		}
		case 3: {
			ofAddListener(window->events().draw, mainApp.get(), &ofApp::drawWindow4);
			break;
		}
				
		default:
			break;
		}
		
	}

	

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();
}
