/* popup.js 
 * This file contains the javascript to open more popups.
 */

function openWindow(url, width, height) {
	var windowname = "popup" + Math.round(Math.random() * 10000, 10000);
	var features = "menubar=no,width=" + width + ",height=" + height + ",screenX=80,screenY=80";
        window.open(url, windowname, features); 
	return false;
}
