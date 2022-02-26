var UserRegionsMixin = function(myPath) {
    // This is a model mixin that manages UI state and server interaction for UserRegions.jsx,
    // the popup in which user can paste or upload user regions.

    'use strict';
    /*jshint validthis: true */

    // If the cartJson response var name changes, change it here too.
    var userRegionsCartVar = 'userRegions';
    var userRegionsUpdateCartVar = 'userRegionsUpdate';
    var summaryCartVar = 'userRegionsSummary';
    var warnCartVar = 'userRegionsWarn';

    var myCartVars = [userRegionsCartVar, userRegionsUpdateCartVar, summaryCartVar, warnCartVar];

    function userRegionsMergeServerResponse(mutState, cartVar, newValue) {
        // Handle server response from setUserRegions command.
        if (cartVar === userRegionsCartVar || cartVar === userRegionsUpdateCartVar) {
            // User regions are small enough that we can pre-fill the region input in case
            // the user wants to edit them.
            mutState.setIn(myPath.concat('regions'), newValue);
        } else if (cartVar === summaryCartVar) {
            mutState.setIn(myPath.concat('summary'), newValue);
        } else if (cartVar === warnCartVar) {
            // Pop up an alert box to show the errors/warnings from parsing the user regions
            alert(newValue);
        } else {
            this.error('UserRegions called for unrecognized cartVar "' + cartVar + '"');
        }
    }

    function openUserRegions(mutState) {
        // Open the user regions popup.
        mutState.setIn(myPath.concat('enabled'), true);
    }

    function closeUserRegions(mutState) {
        // Close the user regions popup.
        mutState.setIn(myPath.concat('enabled'), false);
    }

    function uploadedUserRegions(mutState, uiPath, jqFileInput) {
        var command = { setUserRegions: { regionFileVar: jqFileInput.attr('name'),
                                          resultName: userRegionsUpdateCartVar } };
        this.cartUploadFile(command, jqFileInput);
        closeUserRegions(mutState);
    }

    function pastedUserRegions(mutState, uiPath, newValue) {
        // User pasted & submitted some regions; send to server
        this.cartDo({setUserRegions: { regions: newValue,
                                       resultName: userRegionsUpdateCartVar } });
        closeUserRegions(mutState);
    }

    // Method for use outside this mixin:

    function clearUserRegionState(mutState) {
        // Empty the user-defined regions in mutState and tell server.
        mutState.setIn(myPath.concat('regions'), null);
        mutState.setIn(myPath.concat('summary'), null);
        this.cartDo({setUserRegions: { regions: "",
                                       resultName: userRegionsUpdateCartVar }, });
    }

    function initialize() {
        this.registerCartVarHandler(myCartVars, userRegionsMergeServerResponse);
        this.registerUiHandler(myPath.concat('uploaded'), uploadedUserRegions);
        this.registerUiHandler(myPath.concat('pasted'), pastedUserRegions);
        this.registerUiHandler(myPath.concat('open'), openUserRegions);
        this.registerUiHandler(myPath.concat('cancel'), closeUserRegions);
        this.registerUiHandler(myPath.concat('remove'), closeUserRegions);
        // Install methods for use outside this mixin:
        this.openUserRegions = openUserRegions;
        this.closeUserRegions = closeUserRegions;
        this.clearUserRegionState = clearUserRegionState;
    }

    // Mixin object with initialize
    return { initialize: initialize };
};

// Without this, jshint complains that UserRegionsMixin is not used.  Module system would help.
UserRegionsMixin = UserRegionsMixin;
