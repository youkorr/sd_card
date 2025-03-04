import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM, CONF_FILES
from esphome import automation

DEPENDENCIES = []
CODEOWNERS = ["@votre_nom"]

CONF_STORAGE = "storage"
CONF_DATA = "data"

storage_ns = cg.esphome_ns.namespace('storage')
StorageComponent = storage_ns.class_('StorageComponent', cg.Component)

# Schema for storage configuration
STORAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(StorageComponent),
    cv.Required(CONF_PLATFORM): cv.one_of("flash", "inline", "sd_card", lower=True),
    cv.Optional(CONF_FILES): cv.ensure_list({
        cv.Required(CONF_DATA): cv.returning_lambda,
        cv.Required(CONF_ID): cv.string,
    }),
}).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(
    cv.ensure_list(STORAGE_SCHEMA),
)

def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        yield cg.register_component(var, conf)
        
        cg.add(var.set_platform(conf[CONF_PLATFORM]))
        
        if CONF_FILES in conf:
            for file in conf[CONF_FILES]:
                # Process lambda for data
                data_lambda = yield cg.process_lambda(
                    file[CONF_DATA], 
                    [],  # No arguments
                    return_type=cg.std_string
                )
                
                # Add file with processed lambda
                cg.add(var.add_file(data_lambda, file[CONF_ID]))

# Additional actions and automation registrations remain the same as in the previous version










