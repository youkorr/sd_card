import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", lower=True),
    cv.Required(CONF_FILES): cv.ensure_list({
        cv.Required("source"): cv.string,
        cv.Required("id"): cv.string,
    }),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    cg.add(var.set_platform(config[CONF_PLATFORM]))
    
    for file in config[CONF_FILES]:
        cg.add(var.add_file(file["source"], file["id"]))
