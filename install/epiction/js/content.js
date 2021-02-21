/* content.js 
 * This file contains the javascript to drive the css menus on the content page of the cms
 */

/* Hide all ul elements with 'menu' in the id
 */
function closeAllMenus() {
	var eles = document.getElementsByTagName('ul');
	var i = 0;

	for (i = 0; i < eles.length; i ++) {
		if (eles[i].id.indexOf("menu") >= 0) {
			eles[i].style.display = "none";
		}
	}
}

/* Revert all table rows to unselected and hide context menus
 */
function unselectAll() {
	var eles = document.getElementsByTagName('tr');
	var i = 0;

	for (i = 0; i < eles.length; i ++) {
		if (eles[i].id.indexOf("row") >= 0) {
			if (i % 2) {
				eles[i].style.backgroundColor = "#efefef";
			} else {
				eles[i].style.backgroundColor = "white";
			}
			eles[i].style.color = "#000";
		}
	}
	eles = document.getElementsByTagName('ul');

	for (i = 0; i < eles.length; i ++) {
		if (eles[i].id.indexOf("row") >= 0) {
			eles[i].style.display = "none";
		}
	}
}

/* set white on blue for table row and show context menu */
function setSelection(id) {
	unselectAll();
	var ele = document.getElementById(id);
	ele.style.backgroundColor = "#479";
	ele.style.color = "#fff";
	
	ele = document.getElementById(id + "context");
	ele.style.display = "block";
} 

/* open nav menu */
function setActive(id) {
	closeAllMenus();
	var ele = document.getElementById(id);
	ele.style.display = "block";
} 

/* close nav menu */
function setInactive(id) {
	var ele = document.getElementById(id);
	ele.style.display = "none";
} 

/* if menu is open - close it - else - open menu */
function setToggle(id) {
	var ele = document.getElementById(id);
	if (ele.style.display == "block") {
		ele.style.display = "none";
	} else {
		closeAllMenus();
		ele.style.display = "block";
	}
} 

function setLocation(url) {
	document.location = url;
	return false;
}

function openWindow(url) {
	var windowname = "popup" + Math.round(Math.random() * 10000, 10000);
	var features = "titlebar=yes,menubar=yes,location=yes,screenX=80,screenY=80";
        window.open(url, windowname, features); 
        closeAllMenus();
	return false;
}

function popupWindow(url, width, height) {
	var windowname = "popup" + Math.round(Math.random() * 10000, 10000);
	var features = "menubar=no,width=" + width + ",height=" + height + ",screenX=80,screenY=80";
        window.open(url, windowname, features); 
        closeAllMenus();
	return false;
}
