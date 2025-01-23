const thirst_api = {
  /**
   *  @type {function(string):number}
   **/
  getThirst: ll.import("thirst", "getThirst"),
  /**
   *  @type {function(string,number):void}
   **/
  setThirst: ll.import("thirst", "setThirst"),
  /**
   *  @type {function(string,Boolean):void}
   **/
  setShowThirst: ll.import("thirst", "setShowThirst")
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
 * @returns {number}
 */
function setThirst(uuid, value) {
  return thirst_api.setThirst(uuid, value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {number}
 */
function addThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) + value);
}
/**
 * @param {string} uuid
 * @param {number} value
 * @returns {number}
 */
function reduceThirst(uuid, value) {
  return setThirst(uuid, getThirst(uuid) - value);
}
/**
 * @param {string} uuid
 * @param {Boolean} value
 * @returns {number}
 */
function setThirst(uuid, value) {
  return thirst_api.setShowThirst(uuid, value);
}
