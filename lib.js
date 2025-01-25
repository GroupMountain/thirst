const thirst_api = {
  /**
   *  @type {function(string):number}
   **/
  getThirst: ll.import("thirst", "getThirst"),
  /**
   *  @type {function(string,number):Boolean}
   **/
  setThirst: ll.import("thirst", "setThirst"),
  /**
   *  @type {function(string,Boolean):void}
   **/
  setShowThirst: ll.import("thirst", "setShowThirst"),
  /**
   *  @type {function(string,string,string):string}
   */
  registerOnTickCallback: ll.import("thirst", "registerOnTickCallback")
};
/**
 * @param {string} uuid
 * @returns {number}
 */
function getThirst(uuid) {
  return thirst_api.getThirst(uuid);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {Boolean}
 */
function setThirst(uuid, value) {
  return thirst_api.setThirst(uuid, value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {Boolean}
 */
function addThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) + value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {Boolean}
 */
function reduceThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) - value);
}
/**
 * @param {string} uuid
 * @param {Boolean} value
 * @returns {void}
 */
function setShowThirst(uuid, value) {
  return thirst_api.setShowThirst(uuid, value);
}
/**
 * @param {string} uuid
 * @param {function():void} callback
 * @returns {function():Boolean}
 */
function registerOnTickCallback(uuid, callback) {
  const namespace = Math.random().toString();
  const functionName = Math.random().toString();
  ll.export(namespace, functionName, callback);
  const importName = thirst_api.registerOnTickCallback(
    uuid,
    namespace,
    functionName
  );
  if (importName == "") return null;
  else return ll.import("thirst", importName);
}
