cordova.define('cordova/plugin_list', function(require, exports, module) {
module.exports = [
    {
        "file": "plugins/cordova-plugin-whitelist/whitelist.js",
        "id": "cordova-plugin-whitelist.whitelist",
        "runs": true
    },
    {
        "file": "plugins/org.nypr.cordova.wakeupplugin/www/wakeup.js",
        "id": "org.nypr.cordova.wakeupplugin.Wakeup",
        "clobbers": [
            "wakeuptimer"
        ]
    }
];
module.exports.metadata = 
// TOP OF METADATA
{
    "cordova-plugin-whitelist": "1.2.1",
    "org.nypr.cordova.wakeupplugin": "0.1.0"
};
// BOTTOM OF METADATA
});